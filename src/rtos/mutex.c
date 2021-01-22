/**
 * \file            mutex.c
 * \brief           Mutex source file
 */


/*******************************************************************************************************
 *                         INCLUDE FILES
 ******************************************************************************************************/

#include <stdint.h>

#include "mutex.h"
#include "mutex_forward.h"
#include "os.h"
#include "os_forward.h"


/*******************************************************************************************************
 *                         PRIVATE DEFINES
 ******************************************************************************************************/

/*******************************************************************************************************
 *                         PRIVATE DATA TYPES
 ******************************************************************************************************/

/*******************************************************************************************************
 *                         PRIVATE VARIABLES
 ******************************************************************************************************/

/*******************************************************************************************************
 *                         GLOBAL VARIABLES DEFINITION
 ******************************************************************************************************/

extern OS_TCB_S *OS_TCBCurrent;


/*******************************************************************************************************
 *                         PRIVATE FUNCTIONS DECLARATION
 ******************************************************************************************************/
#ifndef UNIT_TESTING
static 
#endif 
void MutexPendingListAdd(OS_Mutex_S *mutex, OS_TCB_S *tcb);

#ifndef UNIT_TESTING
static 
#endif 
void MutexPendingListRemove(OS_Mutex_S *mutex, OS_TCB_S *tcb);

#ifndef UNIT_TESTING
static 
#endif 
void MutexPendingListRemoveAll(OS_Mutex_S *mutex);


/*******************************************************************************************************
 *                         PUBLIC FUNCTIONS DEFINITION
 ******************************************************************************************************/

/**
  * @brief  		Function which initializes Mutex. PrioInversion parameter sets task priority if 
  * 				priority inversion occurs.
  * @param[in] 		mutex: Mutex handle
  * @param[in] 		mutex: Task priority
  * @param[out] 	mutex: Mutex Error handle
  * @retval 		None
  */
void OS_MutexInit(OS_Mutex_S *mutex, uint32_t prioInversion, OS_MutexError_E *err) {
	OS_MutexError_E errLocal = OS_MUTEX_ERROR_NONE;

	OS_ENTER_CRITICAL();

	if (mutex != NULL) {
		mutex->state = OS_MUTEX_STATE_FREE;
		mutex->owner = NULL;
		mutex->num_of_pending_tasks = 0ul;
		mutex->isInitialized = TRUE;
		mutex->prioInversion = prioInversion;
		mutex->isPrioInversion = FALSE;
	} else {
		errLocal = OS_MUTEX_ERROR_NULL_PTR;
	}

	OS_EXIT_CRITICAL();

	if (err != NULL) {
		*err = errLocal;
	}
}


/**
  * @brief  		Function tries to pend a mutex. If succesfull, it returns
  * 				immediatly. If not, it goes to pending state. It will wait
  * 				in pending state until mutex is freed or timeout pass by.
  * @param[in] 		mutex: Mutex handle
  * @param[in] 		timeout: Timeout value. If 0ul it will wait indefinetly.
  * @param[out] 	mutex: Mutex Error handle
  * @retval 		None
  */
void OS_MutexPend(OS_Mutex_S *mutex, uint32_t timeout, OS_MutexError_E *err) {
	OS_MutexError_E errLocal = OS_MUTEX_ERROR_NONE;
	uint32_t tick;

	OS_ENTER_CRITICAL();

	if (mutex != NULL) {
		if (mutex->isInitialized == TRUE) {
			if (mutex->state == OS_MUTEX_STATE_FREE) {
				mutex->owner = OS_TCBCurrent;
				mutex->owner->mutexPendingNext = NULL;
				mutex->state = OS_MUTEX_STATE_OWNED;

			} else {
				/* if current TCB has higher priority that mutex owner TCB */
				if (OS_TCBCurrent->taskPriority > mutex->owner->taskPriority) {
					mutex->oldOwnerTaskPriority = mutex->owner->taskPriority;
					mutex->owner->taskPriority = mutex->prioInversion;
					mutex->isPrioInversion = TRUE;
				}

				/* add TCB to the mutex pending list */
				MutexPendingListAdd(mutex, OS_TCBCurrent);

				if (timeout > 0ul) {
					tick = OS_getOSTickCounter();

					OS_delayTicks(timeout);
					/* Todo: BUG. If owner task releases mutex, it will
					 * set all pending tasks to the ready state. Again some
					 * mutex that was waiting for task can pend mutex and then
					 * immidiatly call Delay function making new task running. That
					 * task can then go into this if condition because timeout still
					 * hadnt passed by and then, already owned mutex can be taken from
					 * another task.
					 */
					if (OS_getOSTickCounter() - tick < timeout) {
						/* pend it */
						mutex->owner = OS_TCBCurrent;
						mutex->state = OS_MUTEX_STATE_OWNED;

					} else {
						/* timeout time is finished, remove TCB from TCB pending list */
						MutexPendingListRemove(mutex, OS_TCBCurrent);

						errLocal = OS_MUTEX_ERROR_TIMEOUT;
					}

				} else {
					OS_EXIT_CRITICAL();
					OS_Schedule();
					OS_ENTER_CRITICAL();

					mutex->owner = OS_TCBCurrent;
					mutex->state = OS_MUTEX_STATE_OWNED;
				}
			}

		} else {
			errLocal = OS_MUTEX_ERROR_NOT_INITIALIZED;
		}

	} else {
		errLocal = OS_MUTEX_ERROR_NULL_PTR;
	}

	OS_EXIT_CRITICAL();

	if (err != NULL) {
		*err = errLocal;
	}
}


/**
  * @brief  		Function tries to post a mutex. Only owner of the mutex 
  * 				can post a mutex. When mutex it posted, it will resume 
  * 				all the tasks that are waiting for that same mutex.
  * @param[in] 		mutex: Mutex handle
  * @param[out] 	mutex: Mutex Error handle
  * @retval 		None
  */
void OS_MutexPost(OS_Mutex_S *mutex, OS_MutexError_E *err) {
	OS_MutexError_E errLocal = OS_MUTEX_ERROR_NONE;

	if (mutex != NULL) {
		OS_ENTER_CRITICAL();

		if (mutex->isInitialized == TRUE) {
			if (mutex->state == OS_MUTEX_STATE_OWNED) {
				/* only the owner of mutex can post mutex */
				if (mutex->owner == OS_TCBCurrent) {
					mutex->state = OS_MUTEX_STATE_FREE;

					if (mutex->isPrioInversion == TRUE) {
						mutex->isPrioInversion = FALSE;
						mutex->owner->taskPriority = mutex->oldOwnerTaskPriority;
					}

					/* remove all TCBs from mutex pending list */
					MutexPendingListRemoveAll(mutex);

					OS_EXIT_CRITICAL();
					OS_Schedule();
					OS_ENTER_CRITICAL();

				} else {
					errLocal = OS_MUTEX_ERROR_NOT_OWNER_POST;
				}
			} else {
				errLocal = OS_MUTEX_ERROR_NOT_PENDED;
			}
		} else {
			errLocal = OS_MUTEX_ERROR_NOT_INITIALIZED;
		}
	} else {
		errLocal = OS_MUTEX_ERROR_NULL_PTR;
	}

	OS_EXIT_CRITICAL();

	if (err != NULL) {
		*err = errLocal;
	}
}


/*******************************************************************************************************
 *                         PRIVATE FUNCTIONS DEFINITION
 ******************************************************************************************************/

/**
  * @brief  		Function adds task TCB to the waiting list.
  * @param[in] 		mutex: Mutex handle
  * @param[in] 		tcb: Task TCB handle
  * @retval 		None
  */
#ifndef UNIT_TESTING
static 
#endif
void MutexPendingListAdd(OS_Mutex_S *mutex, OS_TCB_S *tcb) {
	OS_TCB_S *i;
	OS_TCB_S *last_i;

	i = mutex->owner->mutexPendingNext;
	last_i = mutex->owner;

	while (i != NULL) {
		last_i = i;
		i = i->mutexPendingNext;
	}
	last_i->mutexPendingNext = tcb;
	last_i->mutexPendingNext->mutexPendingNext = NULL;
	mutex->num_of_pending_tasks++;

	return;
}


/**
  * @brief  		Function removes task TCB from the waiting list.
  * 				TCB must be in the waiting list.
  * @param[in] 		mutex: Mutex handle
  * @param[in] 		tcb: Task TCB handle
  * @retval 		None
  */
#ifndef UNIT_TESTING
static 
#endif
void MutexPendingListRemove(OS_Mutex_S *mutex, OS_TCB_S *tcb) {
	OS_TCB_S *i;
	OS_TCB_S *last_i;

	if (mutex->owner != NULL) {
		i = mutex->owner->mutexPendingNext;
		last_i = mutex->owner;

		while (i != NULL) {
			if (i == tcb) {
				last_i->mutexPendingNext = i->mutexPendingNext;
				mutex->num_of_pending_tasks--;
				break;
			}

			last_i = i;
			i = i->mutexPendingNext;
		}
	}

	tcb->mutexPendingNext = NULL;

	return;
}


/**
  * @brief  		Function removes all the task TCBs from the waiting list.
  *					Also, it will put all the pending tasks in Ready state.
  * @param[in] 		mutex: Mutex handle
  * @retval 		None
  */
#ifndef UNIT_TESTING
static 
#endif
void MutexPendingListRemoveAll(OS_Mutex_S *mutex) {
	OS_TCB_S *i;
	OS_TCB_S *j;

	i = mutex->owner->mutexPendingNext;

	/* TODO: Do we need to set every i->mutexPendingNext to NULL 
	* or just set mutex->owner->mutexPendingNext = NULL??? IvanVnucec
	*/

	while (i != NULL) {
		i->taskState = OS_TASK_STATE_READY;
		i->taskTick = 0ul;
		j = i->mutexPendingNext;
		i = j;
	}
	
	mutex->owner->mutexPendingNext = NULL;
	mutex->num_of_pending_tasks = 0ul;

	return;
}

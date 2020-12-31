/*******************************************************************************************************
 *                         INCLUDE FILES
 ******************************************************************************************************/
#include "mutex.h"
#include "mutex_forward.h"

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

/*******************************************************************************************************
 *                         PRIVATE FUNCTIONS DECLARATION
 ******************************************************************************************************/
void MutexTCBPendingListAdd(OS_Mutex_S *mutex, OS_TCB_S *tcb, uint32_t timeout, uint32_t* error);
void MutexTCBPendingListRemove(OS_Mutex_S *mutex, OS_TCB_S *tcb, uint32_t* error);
void MutexTCBPendingListRemoveAll(OS_Mutex_S *mutex, uint32_t* error);


/*******************************************************************************************************
 *                         PUBLIC FUNCTIONS DEFINITION
 ******************************************************************************************************/
void OS_MutexInit(OS_Mutex_S *mutex, OS_MutexError_E *err) {
	OS_MutexError_E errLocal = OS_MUTEX_ERROR_NONE;

	OS_ENTER_CRITICAL();

	if (mutex != NULL) {
		mutex->state = OS_MUTEX_STATE_FREE;
		mutex->owner = NULL;
		mutex->num_of_pending_tasks = 0ul;

	} else {
		errLocal = OS_MUTEX_ERROR_NULL_PTR;
	}

	OS_EXIT_CRITICAL();

	if (err != NULL) {
		*err = errLocal;
	}
}


void OS_MutexPend(OS_Mutex_S *mutex, uint32_t timeout, OS_MutexError_E *err) {
	OS_MutexError_E errLocal = OS_MUTEX_ERROR_NONE;

	/* TODO: This function needs cleaning IvanVnucec */

	OS_ENTER_CRITICAL();

	if (mutex != NULL) {
		OS_TCBCurrent->mutex = mutex;

		if (mutex->state == OS_MUTEX_STATE_FREE) {
			mutex->state = OS_MUTEX_STATE_OWNED;
			mutex->owner = OS_TCBCurrent;

			OS_EXIT_CRITICAL();

		} else {
			/* put current task to pending for mutex */
			MutexTCBPendingListAdd(mutex, OS_TCBCurrent, timeout, NULL);

			/* call scheduler */
			if (timeout != 0u) {
				OS_EXIT_CRITICAL();
				OS_delayTicks(timeout);
				OS_ENTER_CRITICAL();
			}

			/* we continue here if mutex was freed and the mutex timeout had expired */
			if (OS_TCBCurrent->mutex->state == OS_MUTEX_STATE_FREE && OS_TCBCurrent->taskTick > 0u) {
				/* After a mutex is free again we need to pend it */
				OS_EXIT_CRITICAL();
				/* we can put 0 for timeout because mutex is free here */
				OS_MutexPend(mutex, 0ul, &errLocal);

			} else {
				/* wait indefinitely mode */
				if (timeout == 0ul) {
					while (OS_TCBCurrent->mutex->state == OS_MUTEX_STATE_OWNED) {
						OS_EXIT_CRITICAL();
						OS_Schedule();
						OS_ENTER_CRITICAL();
					}

					OS_EXIT_CRITICAL();

					/* we can put 0 for timeout because mutex is free here */
					OS_MutexPend(mutex, 0ul, &errLocal);

				} else {
					if (OS_TCBCurrent->taskTick > 0u) {
						OS_EXIT_CRITICAL();

						OS_MutexPend(mutex, OS_TCBCurrent->taskTick, &errLocal);
					} else {
						/* remove tcb from mutex tcb pending list because the tcb is
						 * now running (after timeout occured) */
						MutexTCBPendingListRemove(mutex, OS_TCBCurrent, NULL);
						OS_EXIT_CRITICAL();
						errLocal = OS_MUTEX_ERROR_TIMEOUT;
					}
				}
			}
		}
	} else {
		errLocal = OS_MUTEX_ERROR_NULL_PTR;
	}

	if (err != NULL) {
		*err = errLocal;
	}
}


void OS_MutexPost(OS_Mutex_S *mutex, OS_MutexError_E *err) {
	OS_MutexError_E errLocal = OS_MUTEX_ERROR_NONE;

	if (mutex != NULL) {
		OS_ENTER_CRITICAL();

		/* TODO: mutex->owner can be NULL if you call MutexPost IvanVnucec*/
		/* only the owner of mutex can post mutex */
		if (mutex->owner == OS_TCBCurrent) {
			mutex->state = OS_MUTEX_STATE_FREE;
			mutex->owner = NULL;
			MutexTCBPendingListRemoveAll(mutex, NULL);

			OS_EXIT_CRITICAL();

			OS_Schedule();

		} else {
			errLocal = OS_MUTEX_ERROR_NOT_OWNER_POST;
		}

	} else {
		errLocal = OS_MUTEX_ERROR_NULL_PTR;
	}

	if (err != NULL) {
		*err = errLocal;
	}
}


/******************************************** ***********************************************************
 *                         PRIVATE FUNCTIONS DEFINITION
 ******************************************************************************************************/

/* TODO: This functions below should be implemented with lists and not with arrays IvanVnucec*/
void MutexTCBPendingListAdd(OS_Mutex_S *mutex, OS_TCB_S *tcb, uint32_t timeout, uint32_t* error) {
	uint32_t err = 0ul;

	tcb->taskState = OS_TASK_STATE_PENDING;
	//tcb->taskTick = timeout;
	mutex->pending_tasks[mutex->num_of_pending_tasks] = tcb;
	mutex->num_of_pending_tasks++;

	if (error != NULL) {
		*error = err;
	}
}


void MutexTCBPendingListRemove(OS_Mutex_S *mutex, OS_TCB_S *tcb, uint32_t* error) {
	uint32_t err = 0ul;
	uint32_t i;

	if (mutex->num_of_pending_tasks > 0u) {
		/* assumption: we always have variable tcb in mutex->pending_tasks[] array */
		/* find tcb to be removed */
		i = 0ul;
		while (mutex->pending_tasks[i] != tcb) {
			i++;
		}

		/* we dont need to set it to ready state
		 * because it is already in running state */
		/* mutex->pending_tasks[i]->taskState = OS_TASK_STATE_READY; */

		/* assumption: only one tcb needs to be removed */
		/* shift every tcb in list to one place to the left */
		while(i < mutex->num_of_pending_tasks-1) {
			mutex->pending_tasks[i] = mutex->pending_tasks[i+1];
			i++;
		}

		mutex->num_of_pending_tasks--;
	} else {
		err = 1;
	}


	if (error != NULL) {
		*error = err;
	}
}


void MutexTCBPendingListRemoveAll(OS_Mutex_S *mutex, uint32_t* error) {
	uint32_t err = 0ul;
	uint32_t i;

	for (i = 0; i < mutex->num_of_pending_tasks; i++) {
		mutex->pending_tasks[i]->taskState = OS_TASK_STATE_READY;
	}

	mutex->num_of_pending_tasks = 0ul;

	if (error != NULL) {
		*error = err;
	}
}

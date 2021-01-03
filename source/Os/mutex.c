/*******************************************************************************************************
 *                         INCLUDE FILES
 ******************************************************************************************************/
#include <stdint.h>

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
void MutexTCBPendingListAdd(OS_Mutex_S *mutex, OS_TCB_S *tcb, OS_MutexError_E* error);
void MutexTCBPendingListRemove(OS_Mutex_S *mutex, OS_TCB_S *tcb, OS_MutexError_E* error);
void MutexTCBPendingListRemoveAll(OS_Mutex_S *mutex, OS_MutexError_E* error);


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
		mutex->isInitialized = TRUE;

	} else {
		errLocal = OS_MUTEX_ERROR_NULL_PTR;
	}

	OS_EXIT_CRITICAL();

	if (err != NULL) {
		*err = errLocal;
	}
}


void OS_MutexPend(OS_Mutex_S *mutex, OS_MutexError_E *err) {
	OS_MutexError_E errLocal = OS_MUTEX_ERROR_NONE;

	OS_ENTER_CRITICAL();

	if (mutex != NULL) {
		if (mutex->isInitialized == TRUE) {
			if (mutex->state == OS_MUTEX_STATE_FREE) {
				mutex->owner = OS_TCBCurrent;
				mutex->state = OS_MUTEX_STATE_OWNED;

			} else {
				MutexTCBPendingListAdd(mutex, OS_TCBCurrent, &errLocal);

				if (errLocal == OS_MUTEX_ERROR_NONE) {
					OS_Schedule();

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


void OS_MutexPost(OS_Mutex_S *mutex, OS_MutexError_E *err) {
	OS_MutexError_E errLocal = OS_MUTEX_ERROR_NONE;

	if (mutex != NULL) {
		OS_ENTER_CRITICAL();

		/* TODO: mutex->owner can be NULL if you call MutexPost IvanVnucec*/
		if (mutex->owner != NULL) {
			/* only the owner of mutex can post mutex */
			if (mutex->owner == OS_TCBCurrent) {
				mutex->state = OS_MUTEX_STATE_FREE;
				mutex->owner = NULL;
				MutexTCBPendingListRemoveAll(mutex, NULL);

				OS_EXIT_CRITICAL();

				OS_Schedule();

				OS_ENTER_CRITICAL();

			} else {
				errLocal = OS_MUTEX_ERROR_NOT_OWNER_POST;
			}
		} else {
			errLocal = OS_MUTEX_ERROR_NULL_PTR;
		}

	} else {
		errLocal = OS_MUTEX_ERROR_NULL_PTR;
	}

	OS_EXIT_CRITICAL();

	if (err != NULL) {
		*err = errLocal;
	}
}


/******************************************** ***********************************************************
 *                         PRIVATE FUNCTIONS DEFINITION
 ******************************************************************************************************/

/* TODO: This functions below should be implemented with lists and not with arrays IvanVnucec*/
void MutexTCBPendingListAdd(OS_Mutex_S *mutex, OS_TCB_S *tcb, OS_MutexError_E* error) {
	OS_MutexError_E err = OS_MUTEX_ERROR_NONE;

	if (mutex->num_of_pending_tasks < OS_MUTEX_PENDING_TASKS_ARRAY_SIZE) {
		tcb->taskState = OS_TASK_STATE_PENDING;
		mutex->pending_tasks[mutex->num_of_pending_tasks] = tcb;
		mutex->num_of_pending_tasks++;

	} else {
		err = !OS_MUTEX_ERROR_NONE;
	}


	if (error != NULL) {
		*error = err;
	}
}


void MutexTCBPendingListRemove(OS_Mutex_S *mutex, OS_TCB_S *tcb, OS_MutexError_E* error) {
	OS_MutexError_E err = OS_MUTEX_ERROR_NONE;
	uint32_t i;

	if (mutex->num_of_pending_tasks > 0u) {
		i = 0ul;
		while (mutex->pending_tasks[i] != tcb && i < mutex->num_of_pending_tasks) {
			i++;
		}

		/* assumption: only one tcb needs to be removed */
		/* shift every tcb in list to one place to the left */
		while(i < mutex->num_of_pending_tasks-1) {
			mutex->pending_tasks[i] = mutex->pending_tasks[i+1];
			i++;
		}

		mutex->num_of_pending_tasks--;
		tcb->taskState = OS_TASK_STATE_READY;

	} else {
		err = !OS_MUTEX_ERROR_NONE;
	}

	if (error != NULL) {
		*error = err;
	}
}


void MutexTCBPendingListRemoveAll(OS_Mutex_S *mutex, OS_MutexError_E* error) {
	OS_MutexError_E err = OS_MUTEX_ERROR_NONE;
	uint32_t i;

	for (i = 0; i < mutex->num_of_pending_tasks; i++) {
		mutex->pending_tasks[i]->taskState = OS_TASK_STATE_READY;
	}

	mutex->num_of_pending_tasks = 0ul;

	if (error != NULL) {
		*error = err;
	}
}

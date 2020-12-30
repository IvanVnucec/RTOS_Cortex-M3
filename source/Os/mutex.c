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

/*******************************************************************************************************
 *                         PUBLIC FUNCTIONS DEFINITION
 ******************************************************************************************************/
void OS_MutexInit(OS_Mutex_S *mutex, OS_MutexError_E *err) {
	OS_MutexError_E errLocal = OS_MUTEX_ERROR_NONE;

	OS_ENTER_CRITICAL();

	if (mutex != NULL) {
		mutex->state = OS_MUTEX_STATE_FREE;
		mutex->owner = NULL;

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

	if (mutex != NULL) {
		OS_ENTER_CRITICAL();

		OS_TCBCurrent->mutex = mutex;

		if (mutex->state == OS_MUTEX_STATE_FREE) {
			mutex->state = OS_MUTEX_STATE_OWNED;
			mutex->owner = OS_TCBCurrent;

			OS_EXIT_CRITICAL();

		} else {
			/* put current task to pending for mutex */
			OS_TCBCurrent->taskState = OS_TASK_STATE_PENDING;
			OS_TCBCurrent->mutexTimeout = OS_getOSTickCounter() + timeout;
			OS_EXIT_CRITICAL();

			/* call scheduler */
			OS_Schedule();

			OS_ENTER_CRITICAL();

			if (OS_TCBCurrent->mutex->state == OS_MUTEX_STATE_FREE) {
				/* After a mutex is free again we need to pend it */
				OS_EXIT_CRITICAL();
				/* we can put 0 for timeout because mutex is free here */
				OS_MutexPend(mutex, 0ul, &errLocal);

			} else {
				OS_EXIT_CRITICAL();
				errLocal = OS_MUTEX_ERROR_TIMEOUT;
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

		/* only the owner of mutex can post it */
		if (mutex->owner == OS_TCBCurrent) {
			mutex->state = OS_MUTEX_STATE_FREE;
			mutex->owner->mutex = NULL;
			mutex->owner = NULL;
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


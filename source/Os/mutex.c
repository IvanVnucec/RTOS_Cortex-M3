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
void OS_MutexInit(OS_Mutex_S *mutex) {
	OS_ENTER_CRITICAL();

	mutex->state = OS_MUTEX_STATE_FREE;
	mutex->owner = NULL;

	OS_EXIT_CRITICAL();
}


void OS_MutexPend(OS_Mutex_S *mutex) {
	OS_ENTER_CRITICAL();

	OS_TCBCurrent->mutex = mutex;

	if (mutex->state == OS_MUTEX_STATE_FREE) {
		mutex->state = OS_MUTEX_STATE_OWNED;
		mutex->owner = OS_TCBCurrent;

		OS_EXIT_CRITICAL();

	} else {
		/* put current task to pending for mutex */
		OS_TCBCurrent->taskState = OS_TASK_STATE_PENDING;
		OS_EXIT_CRITICAL();

		/* call scheduler */
		OS_Schedule();

		/* After a mutex is free again we need to pend it */
		OS_MutexPend(mutex);
	}
}


void OS_MutexPost(OS_Mutex_S *mutex) {
	OS_ENTER_CRITICAL();

	/* only the owner of mutex can post it */
	if (mutex->owner == OS_TCBCurrent) {
		mutex->state = OS_MUTEX_STATE_FREE;
		mutex->owner->mutex = NULL;
		mutex->owner = NULL;

		OS_Schedule();
	}

	OS_EXIT_CRITICAL();
}


/******************************************** ***********************************************************
 *                         PRIVATE FUNCTIONS DEFINITION
 ******************************************************************************************************/


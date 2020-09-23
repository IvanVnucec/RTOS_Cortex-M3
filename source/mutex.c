#include "mutex.h"
#include "os.h"

extern OS_TCB_S *OS_TCBNext;


void OS_MutexInit(OS_Mutex_E *mutex) {
    if (mutex != (void *)0) {
        *mutex = OS_MUTEX_UNLOCKED;
    }
}


void OS_MutexLock(OS_Mutex_E *mutex) {
    OS_ENTER_CRITICAL();

    if (mutex != NULL) {
        /* if already locked */
        if (*mutex == OS_MUTEX_LOCKED) {
            OS_TCBNext->taskState = OS_TASK_STATE_PENDING;
            OS_TCBNext->mutex = mutex;
            OS_EXIT_CRITICAL();

            OS_Schedule();
        } else {
            *mutex = OS_MUTEX_LOCKED;
            OS_EXIT_CRITICAL();
        }
    }
}


void OS_MutexUnlock(OS_Mutex_E *mutex) {
    OS_ENTER_CRITICAL();

    if (mutex != (void *)0) {
        *mutex = OS_MUTEX_UNLOCKED;
    }

    OS_EXIT_CRITICAL();
}
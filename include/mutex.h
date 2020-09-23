#ifndef MUTEX_H
#define MUTEX_H

typedef enum OS_MUTEX_ENUM {
    OS_MUTEX_UNLOCKED,
    OS_MUTEX_LOCKED
} OS_Mutex_E;

void OS_MutexInit(OS_Mutex_E *mutex);

void OS_MutexLock(OS_Mutex_E *mutex);

void OS_MutexUnlock(OS_Mutex_E *mutex);

#endif /* MUTEX_H */
#include "minunit.h"
#include <string.h>
#include "mutex.h"


OS_TCB_S tcb;
OS_TCB_S *OS_TCBCurrent = &tcb;


void __disable_irq(void) {

}


void __enable_irq(void) {

}


void OS_delayTicks(uint32_t ticks) {
    (void)ticks;
}


void OS_Schedule(void) {
    return;
}


void test_setup(void) {
    memset(OS_TCBCurrent, 0ul, sizeof(*OS_TCBCurrent));
    return;
}


void test_teardown(void) {

    return;
}


MU_TEST(test_OS_MutexInit_nullPtr) {
    OS_MutexError_E err = OS_MUTEX_ERROR_NONE;

    OS_MutexInit(NULL, 1ul, &err);

    mu_assert(err == OS_MUTEX_ERROR_NULL_PTR, "");

}


MU_TEST(test_OS_MutexInit_success) {
    OS_Mutex_S mutex;
    OS_MutexError_E err = !OS_MUTEX_ERROR_NONE;

    OS_MutexInit(&mutex, 1ul, &err);

    mu_assert(mutex.state == OS_MUTEX_STATE_FREE, "");
    mu_assert(mutex.owner == NULL, "");
    mu_assert(mutex.num_of_pending_tasks == 0ul, "");
    mu_assert(mutex.isInitialized == TRUE, "");
    mu_assert(mutex.prioInversion == 1ul, "");
    mu_assert(mutex.isPrioInversion == FALSE, "");
    mu_assert(err == OS_MUTEX_ERROR_NONE, "");
}


MU_TEST(test_OS_MutexPend_nullPtr) {
    OS_MutexError_E err = OS_MUTEX_ERROR_NONE;

    OS_MutexPend(NULL, 1ul, &err);

    mu_assert(err == OS_MUTEX_ERROR_NULL_PTR, 
        "");

}


MU_TEST(test_OS_MutexPend_notInit) {
    OS_Mutex_S mutex;
    OS_MutexError_E err = OS_MUTEX_ERROR_NONE;

    mutex.isInitialized = FALSE;

    OS_MutexPend(&mutex, 1ul, &err);

    mu_assert(err == OS_MUTEX_ERROR_NOT_INITIALIZED, "");
}


MU_TEST(test_OS_MutexPend_mutexFree) {
    OS_Mutex_S mutex;
    OS_MutexError_E err = !OS_MUTEX_ERROR_NONE;

    mutex.isInitialized = TRUE;
    mutex.state = OS_MUTEX_STATE_FREE;

    OS_MutexPend(&mutex, 1ul, &err);
    
    mu_assert(mutex.owner == OS_TCBCurrent, "");
    mu_assert(mutex.owner->mutexPendingNext == NULL, "");
    mu_assert(mutex.state == OS_MUTEX_STATE_OWNED, "");
    mu_assert(err == OS_MUTEX_ERROR_NONE, "");
}


MU_TEST(test_OS_MutexPend_mutexNotFree1) {
    OS_Mutex_S mutex;
    OS_MutexError_E err = !OS_MUTEX_ERROR_NONE;
    OS_TCB_S tcbOwner;

    mutex.isInitialized = TRUE;
    mutex.state = OS_MUTEX_STATE_FREE;
    mutex.owner = &tcbOwner;
    mutex.owner->taskPriority = 2ul;
    OS_TCBCurrent->taskPriority = 3ul;

    OS_MutexPend(&mutex, 1ul, &err);
    
    mu_assert(mutex.owner == OS_TCBCurrent, "");
    mu_assert(mutex.owner->mutexPendingNext == NULL, "");
    mu_assert(mutex.state == OS_MUTEX_STATE_OWNED, "");
    //mu_assert(mutex.oldOwnerTaskPriority ==  mutex.owner->taskPriority, "");
    //mu_assert(mutex.owner->taskPriority == mutex.prioInversion, "");
    //mu_assert(mutex.isPrioInversion == TRUE, "");
    //mu_assert(err == OS_MUTEX_ERROR_NONE, "");
}


MU_TEST_SUITE(test_suite_mutex) {
	MU_SUITE_CONFIGURE(&test_setup, &test_teardown);

	MU_RUN_TEST(test_OS_MutexInit_nullPtr);
    MU_RUN_TEST(test_OS_MutexInit_success);
    MU_RUN_TEST(test_OS_MutexPend_nullPtr);
    MU_RUN_TEST(test_OS_MutexPend_notInit);
    MU_RUN_TEST(test_OS_MutexPend_mutexFree);
    MU_RUN_TEST(test_OS_MutexPend_mutexNotFree1);
}


int main(void) {
	MU_RUN_SUITE(test_suite_mutex);
	MU_REPORT();
    
	return 1;
}

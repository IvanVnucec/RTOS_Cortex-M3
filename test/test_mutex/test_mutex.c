#include "minunit.h"
#include <string.h>
#include "mutex.h"


#define OS_TCB_LIST_LEN 10
OS_TCB_S *tcbList[OS_TCB_LIST_LEN];
uint32_t tcbListIndex;

OS_TCB_S *OS_TCBCurrent;

uint32_t osTicks;


void __disable_irq(void) {

}


void __enable_irq(void) {

}


void OS_delayTicks(uint32_t ticks) {
    osTicks += ticks;
}


void OS_Schedule(void) {

    tcbListIndex++;

    if (tcbListIndex < OS_TCB_LIST_LEN) {
        OS_TCBCurrent = tcbList[tcbListIndex];
    } else {
        tcbListIndex = 0;
    }

    return;
}


void test_setup(void) {
    tcbListIndex = 0ul;
    osTicks = 0ul;

    return;
}


void test_teardown(void) {

    return;
}


/**
 * @note    Mutex pending empty list.
 */
MU_TEST(test_MutexPendingListAdd_success1) {
    OS_TCB_S tcb1, tcb2;
    OS_Mutex_S mutex;
    OS_MutexError_E err = OS_MUTEX_ERROR_NONE;

    tcbList[0] = &tcb1;
    OS_TCBCurrent = tcbList[0];

    OS_MutexInit(&mutex, 10ul, &err);
    OS_MutexPend(&mutex, 10ul, &err);
    MutexPendingListAdd(&mutex, &tcb2);

    mu_assert(mutex.owner->mutexPendingNext == &tcb2,
        "mutex.owner->mutexPendingNext == &tcb2");

    mu_assert(mutex.owner->mutexPendingNext->mutexPendingNext == NULL,
        "mutex.owner->mutexPendingNext->mutexPendingNext == NULL");

    mu_assert(mutex.num_of_pending_tasks == 1ul,
        "mutex.num_of_pending_tasks == 1ul");
}


/**
 * @note    Mutex pending list add multiple.
 */
MU_TEST(test_MutexPendingListAdd_success2) {
    OS_TCB_S tcb1, tcb2, tcb3, tcb4;
    OS_Mutex_S mutex;
    OS_MutexError_E err = OS_MUTEX_ERROR_NONE;

    tcbList[0] = &tcb1;
    OS_TCBCurrent = tcbList[0];

    OS_MutexInit(&mutex, 10ul, &err);
    OS_MutexPend(&mutex, 10ul, &err);
    MutexPendingListAdd(&mutex, &tcb2);
    MutexPendingListAdd(&mutex, &tcb3);
    MutexPendingListAdd(&mutex, &tcb4);

    mu_assert(mutex.owner->mutexPendingNext == &tcb2,
        "mutex.owner->mutexPendingNext == &tcb2");
    
    mu_assert(mutex.owner->mutexPendingNext->mutexPendingNext 
        == &tcb3,
        "mutex.owner->mutexPendingNext == &tcb3");
    
    mu_assert(mutex.owner->mutexPendingNext->mutexPendingNext->mutexPendingNext 
        == &tcb4,
        "mutex.owner->mutexPendingNext == &tcb4");

    mu_assert(mutex.owner->mutexPendingNext->mutexPendingNext->mutexPendingNext->mutexPendingNext 
        == NULL,
        "mutex.owner->mutexPendingNext->mutexPendingNext == NULL");

    mu_assert(mutex.num_of_pending_tasks == 3ul,
        "mutex.num_of_pending_tasks == 3ul");
}


MU_TEST(test_OS_MutexInit_nullPtr) {
    OS_MutexError_E err = OS_MUTEX_ERROR_NONE;

    OS_MutexInit(NULL, 1ul, &err);

    mu_assert(err == OS_MUTEX_ERROR_NULL_PTR,
        "err == OS_MUTEX_ERROR_NULL_PTR");

}


MU_TEST(test_OS_MutexInit_success) {
    OS_TCB_S tcb1;
    OS_Mutex_S mutex;
    OS_MutexError_E err = !OS_MUTEX_ERROR_NONE;

    tcbList[0] = &tcb1;
    OS_TCBCurrent = tcbList[0];

    OS_MutexInit(&mutex, 1ul, &err);

    mu_assert(mutex.state == OS_MUTEX_STATE_FREE, 
        "mutex.state == OS_MUTEX_STATE_FREE");
    mu_assert(mutex.owner == NULL, 
        "mutex.owner == NULL");
    mu_assert(mutex.num_of_pending_tasks == 0ul, 
        "mutex.num_of_pending_tasks == 0ul");
    mu_assert(mutex.isInitialized == TRUE, 
        "mutex.isInitialized == TRUE");
    mu_assert(mutex.prioInversion == 1ul, 
        "mutex.prioInversion == 1ul");
    mu_assert(mutex.isPrioInversion == FALSE, 
        "mutex.isPrioInversion == FALSE");
    mu_assert(err == OS_MUTEX_ERROR_NONE, 
        "err == OS_MUTEX_ERROR_NONE");
}

/**
 * @note Mutex was free
 */
MU_TEST(test_OS_MutexPend_success1) {
    OS_TCB_S tcb1;
    OS_Mutex_S mutex;
    OS_MutexError_E err = !OS_MUTEX_ERROR_NONE;

    tcbList[0] = &tcb1;
    OS_TCBCurrent = tcbList[0];

    OS_MutexInit(&mutex, 1ul, &err);

    OS_MutexPend(&mutex, 1ul, &err);

    mu_assert(mutex.owner == OS_TCBCurrent, 
        "mutex.owner == OS_TCBCurrent");

    mu_assert(mutex.owner->mutexPendingNext == NULL, 
        "mutex.owner->mutexPendingNext == NULL");

    mu_assert(mutex.state == OS_MUTEX_STATE_OWNED, 
        "mutex.state == OS_MUTEX_STATE_OWNED");

    mu_assert(err == OS_MUTEX_ERROR_NONE, 
        "err == OS_MUTEX_ERROR_NONE");
}


/**
 * @note Mutex was not free. Current TCB has got
 *       higher priority that mutex owner. 
 *       TCB waits indefinetly.
 */
MU_TEST(test_OS_MutexPend_success1_5) {
    OS_TCB_S tcb1, tcb2;
    OS_Mutex_S mutex;
    OS_MutexError_E err = !OS_MUTEX_ERROR_NONE;

    /* prepare scheduling in advance */
    tcbList[0] = &tcb1;
    tcbList[1] = &tcb2;
    tcbList[2] = &tcb2;
    OS_TCBCurrent = tcbList[0];

    OS_MutexInit(&mutex, 150ul, &err);

    tcb1.taskPriority = 5ul;
    /* tcb1 pends and owns */
    OS_MutexPend(&mutex, 1ul, &err);

    /* start tcb2 */
    OS_Schedule();

    tcb2.taskPriority = 100ul;
    /* tcb2 tries to pend but waits indefinetly. 
       After some time it is ready again because
       owner task had relised the mutex. */
    OS_MutexPend(&mutex, 0ul, &err);

    mu_assert(mutex.oldOwnerTaskPriority == 5ul, 
        "mutex.oldOwnerTaskPriority == 5ul");

    mu_assert(tcb1.taskPriority == mutex.prioInversion, 
        "tcb1.taskPriority == mutex.prioInversion");

    mu_assert(mutex.isPrioInversion == TRUE, 
        "mutex.isPrioInversion == TRUE");

    mu_assert(mutex.owner == &tcb2, 
        "mutex.owner == &tcb2");

    mu_assert(mutex.state == OS_MUTEX_STATE_OWNED, 
        "mutex.state == OS_MUTEX_STATE_OWNED");

    mu_assert(err == OS_MUTEX_ERROR_NONE, 
        "err == OS_MUTEX_ERROR_NONE");
}


/**
 * @note    Mutex was not free. Current TCB has got
 *          no higher priority than mutex owner.
 *          TCB waits indefinetly.
 */
MU_TEST(test_OS_MutexPend_success2) {
    OS_TCB_S tcb1, tcb2;
    OS_Mutex_S mutex;
    OS_MutexError_E err = !OS_MUTEX_ERROR_NONE;

    /* prepare scheduling in advance */
    tcbList[0] = &tcb1;
    tcbList[1] = &tcb2;
    tcbList[2] = &tcb2;
    OS_TCBCurrent = tcbList[0];

    OS_MutexInit(&mutex, 10ul, &err);

    tcb1.taskPriority = 100ul;
    /* tcb1 pends and owns */
    OS_MutexPend(&mutex, 1ul, &err);

    /* start tcb2 */
    OS_Schedule();

    tcb2.taskPriority = 5ul;
    /* tcb2 tries to pend but waits indefinetly. 
       After some time it is ready again because
       owner task had relised the mutex. */
    OS_MutexPend(&mutex, 0ul, &err);

    mu_assert(mutex.owner == &tcb2, 
        "mutex.owner == OS_TCBCurrent");

    mu_assert(mutex.state == OS_MUTEX_STATE_OWNED, 
        "mutex.state == OS_MUTEX_STATE_OWNED");

    mu_assert(err == OS_MUTEX_ERROR_NONE, 
        "err == OS_MUTEX_ERROR_NONE");

}


/**
 * @note    Mutex was not free. Current TCB has got
 *          no higher priority than mutex owner.
 *          TCB waits for "timeout" time. Timeout time
 *          didnt expire.
 */
MU_TEST(test_OS_MutexPend_success3) {
    OS_TCB_S tcb1, tcb2;
    OS_Mutex_S mutex;
    OS_MutexError_E err = !OS_MUTEX_ERROR_NONE;

    /* prepare scheduling in advance */
    tcbList[0] = &tcb1;
    tcbList[1] = &tcb2;
    tcbList[2] = &tcb2;
    OS_TCBCurrent = tcbList[0];

    OS_MutexInit(&mutex, 200ul, &err);

    tcb1.taskPriority = 100ul;
    /* tcb1 pends and owns */
    OS_MutexPend(&mutex, 1ul, &err);

    /* start tcb2 */
    OS_Schedule();

    tcb2.taskPriority = 5ul;
    /* tcb2 tries to pend but waits for 300ul ticks. 
       After 100 ticks it is ready again because
       owner task had relised the mutex. */
    tcb2.taskTick = 300ul - 100ul;
    OS_MutexPend(&mutex, 300ul, &err);

    mu_assert(mutex.owner == &tcb2, 
        "mutex.owner == OS_TCBCurrent");

    mu_assert(mutex.state == OS_MUTEX_STATE_OWNED, 
        "mutex.state == OS_MUTEX_STATE_OWNED");

    mu_assert(err == OS_MUTEX_ERROR_NONE, 
        "err == OS_MUTEX_ERROR_NONE");

}


/**
 * @note    Mutex was not free. Current TCB has got
 *          no higher priority than mutex owner.
 *          TCB waits for "timeout" time. Timeout time
 *          had expired.
 */
MU_TEST(test_OS_MutexPend_success4) {
    OS_TCB_S tcb1, tcb2;
    OS_Mutex_S mutex;
    OS_MutexError_E err = !OS_MUTEX_ERROR_NONE;

    /* prepare scheduling in advance */
    tcbList[0] = &tcb1;
    tcbList[1] = &tcb2;
    tcbList[2] = &tcb2;
    OS_TCBCurrent = tcbList[0];

    OS_MutexInit(&mutex, 200ul, &err);

    tcb1.taskPriority = 100ul;
    /* tcb1 pends and owns */
    OS_MutexPend(&mutex, 1ul, &err);

    /* start tcb2 */
    OS_Schedule();

    tcb2.taskPriority = 5ul;
    /* tcb2 tries to pend but waits for 300ul ticks. 
       After 100 ticks it is ready again because
       owner task had relised the mutex. */
    tcb2.taskTick = 0ul;
    OS_MutexPend(&mutex, 300ul, &err);

    mu_assert(err == OS_MUTEX_ERROR_TIMEOUT, 
        "err == OS_MUTEX_ERROR_TIMEOUT");

}


/**
 * @note    Mutex handle is NULL
 */
MU_TEST(test_OS_MutexPost_null) {
    OS_MutexError_E err = !OS_MUTEX_ERROR_NONE;

    OS_MutexPost(NULL, &err);

    mu_assert(err == OS_MUTEX_ERROR_NULL_PTR, 
        "err == OS_MUTEX_ERROR_NULL_PTR");
}


/**
 * @note    Mutex not initialized
 */
MU_TEST(test_OS_MutexPost_notInit) {
    OS_TCB_S tcb1;
    OS_Mutex_S mutex;
    OS_MutexError_E err = !OS_MUTEX_ERROR_NONE;

    /* prepare scheduling in advance */
    tcbList[0] = &tcb1;
    OS_TCBCurrent = tcbList[0];

    OS_MutexPost(&mutex, &err);

    mu_assert(err == OS_MUTEX_ERROR_NOT_INITIALIZED, 
        "err == OS_MUTEX_ERROR_NOT_INITIALIZED");
}


/**
 * @note    Mutex not pended
 */
MU_TEST(test_OS_MutexPost_notPended) {
    OS_TCB_S tcb1;
    OS_Mutex_S mutex;
    OS_MutexError_E err = OS_MUTEX_ERROR_NONE;

    /* prepare scheduling in advance */
    tcbList[0] = &tcb1;
    OS_TCBCurrent = tcbList[0];

    OS_MutexInit(&mutex, 10ul, &err);
    OS_MutexPost(&mutex, &err);

    mu_assert(err == OS_MUTEX_ERROR_NOT_PENDED, 
        "err == OS_MUTEX_ERROR_NOT_PENDED");
}


/**
 * @note    Mutex post by non mutex owner.
 */
MU_TEST(test_OS_MutexPost_notOwner) {
    OS_TCB_S tcb1, tcb2;
    OS_Mutex_S mutex;
    OS_MutexError_E err = OS_MUTEX_ERROR_NONE;

    /* prepare scheduling in advance */
    tcbList[0] = &tcb1;
    tcbList[1] = &tcb2;
    OS_TCBCurrent = tcbList[0];

    OS_MutexInit(&mutex, 10ul, &err);
    OS_MutexPend(&mutex, 0ul, &err);
    /* tcb1 -> tcb2 */
    OS_Schedule();
    OS_MutexPost(&mutex, &err);

    mu_assert(err == OS_MUTEX_ERROR_NOT_OWNER_POST, 
        "err == OS_MUTEX_ERROR_NOT_OWNER_POST");
}


/**
 * @note    Mutex post with priority inversion
 */
MU_TEST(test_OS_MutexPost_success1) {
    OS_TCB_S tcb1, tcb2;
    OS_Mutex_S mutex;
    OS_MutexError_E err = OS_MUTEX_ERROR_NONE;
    
    /* prepare scheduling in advance */
    tcbList[0] = &tcb1; 
    tcbList[1] = &tcb2;
    tcbList[2] = &tcb1;
    tcbList[3] = &tcb1;
    OS_TCBCurrent = tcbList[0];

    OS_MutexInit(&mutex, 100ul, &err);

    tcb1.taskPriority = 5ul;
    OS_MutexPend(&mutex, 0ul, &err);

    /* tcb1 -> tcb2 */
    OS_Schedule();

    tcb2.taskPriority = 10ul;
    OS_MutexPend(&mutex, 0ul, &err);

    OS_MutexPost(&mutex, &err);

    mu_assert(mutex.isPrioInversion == FALSE, 
        "mutex.isPrioInversion == FALSE");

    mu_assert(mutex.owner->taskPriority == mutex.oldOwnerTaskPriority, 
        "mutex.owner->taskPriority == mutex.oldOwnerTaskPriority");

    mu_assert(err == OS_MUTEX_ERROR_NONE, 
        "err == OS_MUTEX_ERROR_NONE");
}


/**
 * @note    Mutex post without priority inversion
 */
MU_TEST(test_OS_MutexPost_success2) {
    OS_TCB_S tcb1, tcb2;
    OS_Mutex_S mutex;
    OS_MutexError_E err = OS_MUTEX_ERROR_NONE;
    

    /* prepare scheduling in advance */
    tcbList[0] = &tcb1; 
    tcbList[1] = &tcb2;
    tcbList[2] = &tcb1;
    tcbList[3] = &tcb1;
    OS_TCBCurrent = tcbList[0];

    OS_MutexInit(&mutex, 100ul, &err);

    tcb1.taskPriority = 100ul;
    OS_MutexPend(&mutex, 0ul, &err);

    /* tcb1 -> tcb2 */
    OS_Schedule();

    tcb2.taskPriority = 5ul;
    OS_MutexPend(&mutex, 0ul, &err);

    OS_MutexPost(&mutex, &err);

    mu_assert(err == OS_MUTEX_ERROR_NONE, 
        "err == OS_MUTEX_ERROR_NONE");
}


/* TODO: ADD MutexPendingListAdd(mutex, OS_TCBCurrent);
    and other like functions unit tests. IvanVnucec
*/

MU_TEST_SUITE(test_suite_mutex) {
	MU_SUITE_CONFIGURE(&test_setup, &test_teardown);

    MU_RUN_TEST(test_MutexPendingListAdd_success1);
    MU_RUN_TEST(test_MutexPendingListAdd_success2);

	MU_RUN_TEST(test_OS_MutexInit_nullPtr);
    MU_RUN_TEST(test_OS_MutexInit_success);

    MU_RUN_TEST(test_OS_MutexPend_success1);
    MU_RUN_TEST(test_OS_MutexPend_success1_5);
    MU_RUN_TEST(test_OS_MutexPend_success2);
    MU_RUN_TEST(test_OS_MutexPend_success3);
    MU_RUN_TEST(test_OS_MutexPend_success4);

    MU_RUN_TEST(test_OS_MutexPost_null);
    MU_RUN_TEST(test_OS_MutexPost_notInit);
    MU_RUN_TEST(test_OS_MutexPost_notPended);
    MU_RUN_TEST(test_OS_MutexPost_notOwner);
    MU_RUN_TEST(test_OS_MutexPost_success1);
    MU_RUN_TEST(test_OS_MutexPost_success2);
}


int main(void) {
	MU_RUN_SUITE(test_suite_mutex);
	MU_REPORT();
    
	return MU_EXIT_CODE;
}

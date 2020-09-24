#include <stdint.h>
#include "os.h"


#define NVIC_INT_CTRL *((uint32_t volatile *)0xE000ED04)
#define NVIC_PENDSVSET_MASK                  0x10000000

#define SIZEOF_TASKIDLESTACK 256


static void OS_TriggerContextSwitch(void);
static void OS_TaskIdle(void);


uint32_t OS_tickCounter;

OS_TCB_S *OS_TCBList[64];
uint32_t OS_TCBItemsInList;
uint32_t OS_TCBCurrentIndex;
uint32_t OS_TCBNextIndex;
OS_TCB_S *OS_TCBCurrent;
OS_TCB_S *OS_TCBNext;


OS_TCB_S taskIdleTCB;
uint32_t taskIdleStack[SIZEOF_TASKIDLESTACK];


void OS_TaskCreate(OS_TCB_S *taskTCB, 
                void (*taskPointer)(void), 
                uint32_t taskPriority,
                uint8_t *taskName,
                uint32_t *taskStack, 
                uint32_t taskStackSize) {
    OS_ENTER_CRITICAL();

    taskTCB->sp = taskStack + taskStackSize;

    taskTCB->sp--;
    *taskTCB->sp = 0x21000000;

    taskTCB->sp--;
    *taskTCB->sp = (uint32_t)taskPointer;

    taskTCB->sp--;
    *taskTCB->sp = 0x00000004;

    taskTCB->sp--;
    *taskTCB->sp = 0x00000012;

    taskTCB->sp--;
    *taskTCB->sp = 0x00000003;

    taskTCB->sp--;
    *taskTCB->sp = 0x00000002;

    taskTCB->sp--;
    *taskTCB->sp = 0x00000001;

    taskTCB->sp--;
    *taskTCB->sp = 0x00000000;

    taskTCB->sp--;
    *taskTCB->sp = 0x00000011;

    taskTCB->sp--;
    *taskTCB->sp = 0x00000010;

    taskTCB->sp--;
    *taskTCB->sp = 0x00000009;

    taskTCB->sp--;
    *taskTCB->sp = 0x00000008;

    taskTCB->sp--;
    *taskTCB->sp = 0x00000007;

    taskTCB->sp--;
    *taskTCB->sp = 0x00000006;

    taskTCB->sp--;
    *taskTCB->sp = 0x00000005;

    taskTCB->sp--;
    *taskTCB->sp = 0x00000004;

    taskTCB->taskState = OS_TASK_STATE_READY;
    taskTCB->taskPriority = taskPriority;
    taskTCB->taskTick = (uint32_t)0;
    taskTCB->taskName = taskName;
    taskTCB->mutex = NULL;

    OS_TCBList[OS_TCBItemsInList] = taskTCB;
    OS_TCBItemsInList++;

    if (OS_TCBItemsInList > 64) {
        OS_TCBItemsInList = 0;
    }

    OS_EXIT_CRITICAL();
}


void OS_Schedule(void) {
    uint32_t i;
    uint32_t taskMaxPriorityIndex;

    taskMaxPriorityIndex = 0ul;

    OS_ENTER_CRITICAL();


    OS_TCBCurrentIndex = OS_TCBNextIndex;
    OS_TCBCurrent = OS_TCBList[OS_TCBCurrentIndex];

    if (OS_TCBCurrent->taskState == OS_TASK_STATE_RUNNING) {
        OS_TCBCurrent->taskState = OS_TASK_STATE_READY;
    }

    for (i = 0ul; i < OS_TCBItemsInList; i++) {
        if (OS_TCBList[i]->taskTick == 0ul) {
            OS_TCBList[i]->taskState = OS_TASK_STATE_READY;

            if (OS_TCBList[i]->mutex != NULL) {
                if (*OS_TCBList[i]->mutex == OS_MUTEX_UNLOCKED) {
                    OS_TCBList[i]->taskState = OS_TASK_STATE_READY;
                } else {
                    OS_TCBList[i]->taskState = OS_TASK_STATE_PENDING;
                }
            }
        }

        if (OS_TCBList[i]->taskTick > 0ul) {
            OS_TCBList[i]->taskTick--;
        }

        /* choose a thread to run next */
        if (OS_TCBList[i]->taskState == OS_TASK_STATE_READY && 
            OS_TCBList[i]->taskPriority < OS_TCBList[taskMaxPriorityIndex]->taskPriority) {
            taskMaxPriorityIndex = i;
        }
    }
    
    OS_TCBNextIndex = taskMaxPriorityIndex;

    OS_TCBNext = OS_TCBList[OS_TCBNextIndex];
    OS_TCBNext->taskState = OS_TASK_STATE_RUNNING;



    OS_EXIT_CRITICAL();

    OS_TriggerContextSwitch();
}


void OS_Init(void) {
    OS_TCBItemsInList = 0;
    OS_TCBCurrentIndex = 0;
    OS_TCBNextIndex = 0;

    OS_TaskCreate(&taskIdleTCB, 
              &OS_TaskIdle, 
              63ul,
              "taskIdle",
              taskIdleStack, 
              SIZEOF_TASKIDLESTACK);

    OS_TCBCurrent = (OS_TCB_S *)0;
    OS_TCBNext = OS_TCBList[0];
    OS_TCBNext->taskState = OS_TASK_STATE_RUNNING;
}


void OS_Start(void) {

    OS_TriggerContextSwitch();
}


uint32_t OS_getOSTickCounter(void) {
    return OS_tickCounter;
}


void OS_delayTicks(uint32_t ticks) {

    OS_ENTER_CRITICAL();
    
    OS_TCBNext->taskTick = ticks;
    OS_TCBNext->taskState = OS_TASK_STATE_PENDING;

    OS_EXIT_CRITICAL();

    OS_Schedule();

    /*
    tick = OS_getOSTickCounter();

    while (OS_getOSTickCounter() - tick < ticks);
    */
   
}


static void OS_TriggerContextSwitch(void) {
    NVIC_INT_CTRL = NVIC_PENDSVSET_MASK;
}


static void OS_TaskIdle(void) {
    for (;;)
        ;
}


void SysTick_Handler(void) {
    OS_tickCounter++;
    OS_Schedule();
}







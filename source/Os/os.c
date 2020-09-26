/*******************************************************************************************************
 *                         INCLUDE FILES
 ******************************************************************************************************/
#include <stdint.h>
#include "os.h"


/*******************************************************************************************************
 *                         PRIVATE DEFINES
 ******************************************************************************************************/
#define NVIC_INT_CTRL *((uint32_t volatile *)0xE000ED04)
#define NVIC_PENDSVSET_MASK                  0x10000000

#define SIZEOF_TASKIDLESTACK (256)



#define OS_1MILISECOND_TO_TICKS	(OS_MS_TO_TICKS(1ul))
#define OS_1SECOND_TO_TICKS		(1000ul * OS_1MILISECOND_TO_TICKS)
#define OS_1MINUTE_TO_TICKS 	(60ul * OS_1SECOND_TO_TICKS)
#define OS_1HOUR_TO_TICKS		(60ul * OS_1MINUTE_TO_TICKS)
#define OS_1DAY_TO_TICKS		(24ul * OS_1HOUR_TO_TICKS)


/*******************************************************************************************************
 *                         PRIVATE DATA TYPES
 ******************************************************************************************************/

/*******************************************************************************************************
 *                         PRIVATE VARIABLES
 ******************************************************************************************************/
uint32_t OS_tickCounter;

OS_TCB_S *OS_TCBList[64];
uint32_t OS_TCBItemsInList;
uint32_t OS_TCBCurrentIndex;
uint32_t OS_TCBNextIndex;
OS_TCB_S *OS_TCBCurrent;
OS_TCB_S *OS_TCBNext;


OS_TCB_S taskIdleTCB;
uint32_t taskIdleStack[SIZEOF_TASKIDLESTACK];


/*******************************************************************************************************
 *                         GLOBAL VARIABLES DEFINITION
 ******************************************************************************************************/

/*******************************************************************************************************
 *                         PRIVATE FUNCTIONS DECLARATION
 ******************************************************************************************************/
static void OS_TriggerContextSwitch(void);
static void OS_TaskIdle(void);


/*******************************************************************************************************
 *                         PUBLIC FUNCTIONS DEFINITION
 ******************************************************************************************************/
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

    /* If the current task was running then set it to ready state */
    if (OS_TCBCurrent->taskState == OS_TASK_STATE_RUNNING) {
    	OS_TCBCurrent->taskState = OS_TASK_STATE_READY;
    }

    for (i = 0ul; i < OS_TCBItemsInList; i++) {
        /* choose a thread to run next based on threads priority*/
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
              (uint8_t *)"taskIdle",
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
    
    OS_TCBCurrent->taskTick = ticks;
    OS_TCBCurrent->taskState = OS_TASK_STATE_PENDING;

    OS_EXIT_CRITICAL();

    OS_Schedule();
   
}


void OS_delayTime(uint32_t days,
		uint32_t hours,
		uint32_t minutes,
		uint32_t seconds,
		uint32_t miliseconds) {

	OS_delayTicks(days * OS_1DAY_TO_TICKS +
			hours * OS_1HOUR_TO_TICKS +
			minutes * OS_1MINUTE_TO_TICKS +
			seconds * OS_1SECOND_TO_TICKS +
			miliseconds * OS_1MILISECOND_TO_TICKS);
}


void SysTick_Handler(void) {
	int i;

	OS_ENTER_CRITICAL();

    OS_tickCounter++;

    for (i = 0ul; i < OS_TCBItemsInList; i++) {
    	/* If there are pending tasks */
    	if (OS_TCBList[i]->taskState == OS_TASK_STATE_PENDING) {
    		/* Decrement tick delay or set task ready if delay expired */
			if (OS_TCBList[i]->taskTick > 0ul) {
				OS_TCBList[i]->taskTick--;
			} else {
				OS_TCBList[i]->taskState = OS_TASK_STATE_READY;
			}
    	}
    }

    OS_EXIT_CRITICAL();

    OS_Schedule();
}


/*******************************************************************************************************
 *                         PRIVATE FUNCTIONS DEFINITION
 ******************************************************************************************************/
static void OS_TriggerContextSwitch(void) {
    NVIC_INT_CTRL = NVIC_PENDSVSET_MASK;
}


static void OS_TaskIdle(void) {
    for (;;)
        ;
}








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

OS_TCB_S *OS_TCBList[OS_TCB_LIST_LENGTH];
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
                uint32_t taskStackSize,
				OS_Error_E *err) {

	OS_Error_E errLocal = OS_ERROR_NONE;

    OS_ENTER_CRITICAL();

    taskTCB->sp = taskStack + taskStackSize;

    taskTCB->sp--;
    *taskTCB->sp = 0x21000000;

    taskTCB->sp--;
    *taskTCB->sp = (uint32_t)taskPointer;

    taskTCB->sp--;
    *taskTCB->sp = (uint32_t)&OS_TaskTerminate;

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
    taskTCB->taskTick = 0ul;
    taskTCB->lockedByTick = FALSE;
    taskTCB->taskName = taskName;

    /* if there is no room in OS_TCBList */
    if (OS_TCBItemsInList >= OS_TCB_LIST_LENGTH) {
        errLocal = OS_ERROR_TASK_NOT_CREATED;
    }

    if (errLocal == OS_ERROR_NONE) {
    	OS_TCBList[OS_TCBItemsInList] = taskTCB;
    	OS_TCBItemsInList++;
    }

    OS_EXIT_CRITICAL();

    if (err != NULL) {
    	*err = errLocal;
    }
}


void OS_Schedule(void) {
    uint32_t i;
    uint32_t taskMaxPriorityIndex;

    taskMaxPriorityIndex = 0ul;

	OS_ENTER_CRITICAL();

    /* If the current task was still running then set it to ready state */
    if (OS_TCBCurrent->taskState == OS_TASK_STATE_RUNNING) {
    	OS_TCBCurrent->taskState = OS_TASK_STATE_READY;
    }

    for (i = 0ul; i < OS_TCBItemsInList; i++) {
		if (OS_TCBList[i]->taskTick == 0ul && OS_TCBList[i]->lockedByTick == TRUE) {
			OS_TCBList[i]->taskState = OS_TASK_STATE_READY;
		}

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


void OS_Init(OS_Error_E *err) {
	OS_Error_E errLocal = OS_ERROR_NONE;

    OS_TCBItemsInList = 0ul;
    OS_TCBCurrentIndex = 0ul;
    OS_TCBNextIndex = 0ul;

    OS_TaskCreate(&taskIdleTCB, 
              &OS_TaskIdle, 
			  OS_IDLE_TASK_PRIORITY,
              (uint8_t *)"taskIdle",
              taskIdleStack, 
              SIZEOF_TASKIDLESTACK,
			  &errLocal);

    if (errLocal == OS_ERROR_NONE) {
    	OS_TCBCurrent = (OS_TCB_S *)0;
    	OS_TCBNext = OS_TCBList[0];
    	OS_TCBNext->taskState = OS_TASK_STATE_RUNNING;
    }

    if (err != NULL) {
    	*err = errLocal;
    }
}


void OS_Start(OS_Error_E *err) {
	OS_Error_E errLocal = OS_ERROR_NONE;

    OS_TriggerContextSwitch();

    if (err != NULL) {
    	*err = errLocal;
    }
}


uint32_t OS_getOSTickCounter(void) {
    return OS_tickCounter;
}


void OS_delayTicks(uint32_t ticks) {
    OS_ENTER_CRITICAL();
    
    OS_TCBCurrent->taskTick = ticks;
    OS_TCBCurrent->lockedByTick = TRUE;
    OS_TCBCurrent->taskState = OS_TASK_STATE_PENDING;

    OS_EXIT_CRITICAL();

    OS_Schedule();

    OS_ENTER_CRITICAL();

    OS_TCBCurrent->lockedByTick = FALSE;

    OS_EXIT_CRITICAL();
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


void OS_TaskTerminate(void) {
	OS_ENTER_CRITICAL();

	OS_TCBCurrent->taskState = OS_TASK_STATE_DORMANT;

	OS_EXIT_CRITICAL();

	OS_Schedule();
}


void SysTick_Handler(void) {
	uint32_t i;

	OS_ENTER_CRITICAL();

    OS_tickCounter++;

    for(i = 0ul; i < OS_TCBItemsInList; i++) {
    	if (OS_TCBList[i] != NULL) {
        	if (OS_TCBList[i]->taskTick > 0ul) {
        		OS_TCBList[i]->taskTick--;
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








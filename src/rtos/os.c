/**
 * \file            os.c
 * \brief           OS source file
 */


/*******************************************************************************************************
 *                         INCLUDE FILES
 ******************************************************************************************************/
#include "os.h"
#include "os_errors.h"


/*******************************************************************************************************
 *                         PRIVATE DEFINES
 ******************************************************************************************************/
#define NVIC_INT_CTRL *((uint32_t volatile *)0xE000ED04)
#define NVIC_PENDSVSET_MASK                  0x10000000

#define SIZEOF_TASKIDLESTACK (256UL)

#define OS_1MILISECOND_TO_TICKS	(OS_MS_TO_TICKS(1ul))
#define OS_1SECOND_TO_TICKS		(1000ul * OS_1MILISECOND_TO_TICKS)
#define OS_1MINUTE_TO_TICKS 	(60ul * OS_1SECOND_TO_TICKS)
#define OS_1HOUR_TO_TICKS		(60ul * OS_1MINUTE_TO_TICKS)


/*******************************************************************************************************
 *                         PRIVATE DATA TYPES
 ******************************************************************************************************/

/*******************************************************************************************************
 *                         PRIVATE VARIABLES
 ******************************************************************************************************/
uint32_t OS_tickCounter;

OS_TCB_S *OS_TCBListHead;
uint32_t OS_TCBItemsInList;
OS_TCB_S *OS_TCBCurrent;
OS_TCB_S *OS_TCBNext;

static uint32_t OS_schedEnabled;

static OS_TCB_S taskIdleTCB;
static uint32_t taskIdleStack[SIZEOF_TASKIDLESTACK];


/*******************************************************************************************************
 *                         GLOBAL VARIABLES DEFINITION
 ******************************************************************************************************/

/*******************************************************************************************************
 *                         PRIVATE FUNCTIONS DECLARATION
 ******************************************************************************************************/
static void OS_TriggerContextSwitch(void);
static void OS_TaskIdle(void *param);


/*******************************************************************************************************
 *                         PUBLIC FUNCTIONS DEFINITION
 ******************************************************************************************************/

/**
  * @brief  		Function creates a task. After creation it calls Scheduler.
  * @param[in] 	    taskTCB: Task TCB handle
  * @param[in] 		taskPointer: Task function pointer
  * @param[in]      taskPriority: Task priority. Higher the number, higher the priority.
  * @param[in]      taskName: Task name. Used for debugging
  * @param[in]      taskStack: Task available task stack pointer.
  * @param[in]      taskStackSize: Task available task stack size.
  * @param[out] 	err: OS Error handle
  * @retval 		None
  */
void OS_TaskCreate(OS_TCB_S *taskTCB, 
                    void (*taskPointer)(void *), 
                    uint32_t taskPriority,
                    uint8_t *taskName,
                    uint32_t *taskStack, 
                    uint32_t taskStackSize,
                    OS_Error_E *err) {

	  OS_Error_E errLocal = OS_ERROR_NONE;
    OS_TCB_S *i;

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
    taskTCB->taskName = taskName;
    taskTCB->TCBNext = NULL;
    taskTCB->mutexPendingNext = NULL;

    /* skip if no items in linked list */
    if (OS_TCBListHead != NULL) {
        i = OS_TCBListHead;

        while (i->TCBNext != NULL) {
            i = i->TCBNext;
        }

        i->TCBNext = taskTCB;

    } else {
        OS_TCBListHead = taskTCB;
    }

    OS_TCBItemsInList++;

    OS_EXIT_CRITICAL();

    if (errLocal == OS_ERROR_NONE) {
    	OS_Schedule();
    }

    if (err != NULL) {
    	*err = errLocal;
    }
}


/**
  * @brief  		Chooses highest priority ready task.
  * @param          None
  * @retval 		None
  */
void OS_Schedule(void) {
    OS_TCB_S *maxPriorityTask;
    OS_TCB_S *i;

	OS_ENTER_CRITICAL();

	if (OS_schedEnabled == TRUE) {
		/* If the current task was still running then set it to ready state */
        /* this must be checked because OS_TCBCurrent can be NULL */
        if (OS_TCBCurrent != NULL) {
            if (OS_TCBCurrent->taskState == OS_TASK_STATE_RUNNING) {
                OS_TCBCurrent->taskState = OS_TASK_STATE_READY;
            }
        }

        /* choose a thread to run next based on threads priority*/
        maxPriorityTask = &taskIdleTCB;
        /* assumption: Idle taks is first in TCB linked list */
        i = OS_TCBListHead->TCBNext;
		while (i != NULL) {
			if ((i->taskState == OS_TASK_STATE_READY) && (i->taskPriority > maxPriorityTask->taskPriority)) {
				maxPriorityTask = i;
			}

            i = i->TCBNext;
		}

		OS_TCBNext = maxPriorityTask;
        OS_TCBNext->taskState = OS_TASK_STATE_RUNNING;

		OS_EXIT_CRITICAL();

		OS_TriggerContextSwitch();
	}

	OS_EXIT_CRITICAL();

}


/**
  * @brief  		Initialization of the OS. Must be called before any
  *                 other OS function.
  * @param[out] 	err: OS Error handle
  * @retval 		None
  */
void OS_Init(OS_Error_E *err) {
	OS_Error_E errLocal = OS_ERROR_NONE;

	OS_schedEnabled = FALSE;

    OS_TCBItemsInList = 0ul;
    OS_TCBListHead = NULL;
    OS_TCBCurrent = NULL;

    OS_TaskCreate(&taskIdleTCB, 
      &OS_TaskIdle, 
      OS_IDLE_TASK_PRIORITY,
      (uint8_t *)"taskIdle",
      taskIdleStack, 
      SIZEOF_TASKIDLESTACK,
      &errLocal);

    if (err != NULL) {
    	*err = errLocal;
    }
}


/**
  * @brief  		Enables context switching and calls the scheduler.
  * @param[out] 	err: OS Error handle
  * @retval 		None
  */
void OS_EnableScheduler(OS_Error_E *err) {
	OS_Error_E errLocal = OS_ERROR_NONE;

	OS_ENTER_CRITICAL();
	OS_schedEnabled = TRUE;
	OS_EXIT_CRITICAL();

	OS_Schedule();

    if (err != NULL) {
    	*err = errLocal;
    }
}


/**
  * @brief  		Disables context switching. Can be used to guard
  *                 shared resources between tasks.
  * @param[out] 	err: OS Error handle
  * @retval 		None
  */
void OS_DisableScheduler(OS_Error_E *err) {
	OS_Error_E errLocal = OS_ERROR_NONE;

	OS_ENTER_CRITICAL();
	OS_schedEnabled = FALSE;
	OS_EXIT_CRITICAL();

    if (err != NULL) {
    	*err = errLocal;
    }
}


/**
  * @brief  		Returns OS ticks counter.
  * @param       	None
  * @retval 		OS ticks counter.
  */
uint32_t OS_getOSTickCounter(void) {
    return OS_tickCounter;
}


/**
  * @brief  		Delays caller task by number of ticks.
  *                 To convert milieconds to ticks use OS_MS_TO_TICKS(x)
  *                 function macro.
  * @retval 		None
  */
void OS_delayTicks(uint32_t ticks) {
    OS_ENTER_CRITICAL();
    
    OS_TCBCurrent->taskTick = ticks;
    OS_TCBCurrent->taskState = OS_TASK_STATE_PENDING;

    OS_EXIT_CRITICAL();

    OS_Schedule();
}


/**
  * @brief  		Delay a caller task.
  * @param[in]    	hours
  * @param[in]    	minutes
  * @param[in]    	seconds
  * @param[in]    	miliseconds
  * @retval 		None
  */
void OS_delayTime(uint32_t hours,
		uint32_t minutes,
		uint32_t seconds,
		uint32_t miliseconds) {

	OS_delayTicks(hours * OS_1HOUR_TO_TICKS +
			minutes * OS_1MINUTE_TO_TICKS +
			seconds * OS_1SECOND_TO_TICKS +
			miliseconds * OS_1MILISECOND_TO_TICKS);
}


/**
  * @brief  		Put the caller task to the Dormant state.
  * @param          None
  * @retval 		None
  */
void OS_TaskTerminate(void) {
	OS_ENTER_CRITICAL();

	OS_TCBCurrent->taskState = OS_TASK_STATE_DORMANT;

	OS_EXIT_CRITICAL();

	OS_Schedule();
}


/**
  * @brief  		Function increments the OS tick counter. 
  *                 Decresses delay time for every task waiting
  *                 for delay to come to an end. 
  * @param          None
  * @retval 		None
  */
void OS_TickHandler(void) {
	OS_TCB_S *i;

	OS_ENTER_CRITICAL();

    OS_tickCounter++;

    /* assumption: Idle task is first in TCB list */
    i = OS_TCBListHead->TCBNext;
    while(i != NULL)  {
        if (i->taskTick > 0ul) {
            i->taskTick--;

            if (i->taskTick == 0u) {
                i->taskState = OS_TASK_STATE_READY;
            }
        }

        i = i->TCBNext;
    }

    OS_EXIT_CRITICAL();

    OS_Schedule();
}


/**
  * @brief  		System tick handler 
  * @param          None
  * @retval 		None
  */
void SysTick_Handler(void) {
	OS_TickHandler();
}


/*******************************************************************************************************
 *                         PRIVATE FUNCTIONS DEFINITION
 ******************************************************************************************************/
/**
  * @brief  		Generates software (PENDSV) interrupt. 
  * @param          None
  * @retval 		None
  */
static void OS_TriggerContextSwitch(void) {
    NVIC_INT_CTRL = NVIC_PENDSVSET_MASK;
}


/**
  * @brief  		OS Idle task. 
  * @param          None
  * @retval 		None
  */
static void OS_TaskIdle(void *param) {
    for (;;)
        ;
}








#ifndef OS_H
#define OS_H


#include <stdint.h>
#include "mutex.h"


#define TRUE (1ul)
#define FALSE (0ul)

#define NULL  ((void *)0)

#define OS_ENTER_CRITICAL() ({asm ("CPSID I");})
#define OS_EXIT_CRITICAL()  ({asm ("CPSIE I");})


extern void PendSV_Handler(void);


typedef enum OS_TaskState_ENUM {
    OS_TASK_STATE_DORMANT,
    OS_TASK_STATE_READY,
    OS_TASK_STATE_RUNNING,
    OS_TASK_STATE_PENDING
} OS_TaskState_E;


typedef struct OS_TCB_STRUCT {
    uint32_t *sp;
    OS_TaskState_E taskState;
    uint32_t taskPriority;
    uint32_t taskTick;
    uint8_t *taskName;
    OS_Mutex_E *mutex;
} OS_TCB_S;


extern void SysTick_Handler(void);


void OS_TaskCreate(OS_TCB_S *taskTCB, 
                void (*taskPointer)(void), 
                uint32_t taskPriority, 
                uint8_t *taskName,
                uint32_t *taskStack, 
                uint32_t taskStackSize);


void OS_Schedule(void);

void OS_Init(void);

void OS_Start(void);

uint32_t OS_getOSTickCounter(void);

void OS_delayTicks(uint32_t ticks);



#endif /* #ifndef OS_H */

/*******************************************************************************************************
 *                         INCLUDE FILES
 ******************************************************************************************************/
#include "main.h"
#include "diag/Trace.h"

/*******************************************************************************************************
 *                         PRIVATE DEFINES
 ******************************************************************************************************/

/*******************************************************************************************************
 *                         PRIVATE DATA TYPES
 ******************************************************************************************************/

/*******************************************************************************************************
 *                         PRIVATE VARIABLES
 ******************************************************************************************************/
static uint32_t task1Stack[256ul];
static uint32_t task2Stack[256ul];
static uint32_t task3Stack[256ul];

static OS_TCB_S task1TCB;
static OS_TCB_S task2TCB;
static OS_TCB_S task3TCB;

uint32_t cnt;

OS_Mutex_S mutex1;


/*******************************************************************************************************
 *                         GLOBAL VARIABLES DEFINITION
 ******************************************************************************************************/

/*******************************************************************************************************
 *                         PRIVATE FUNCTIONS DECLARATION
 ******************************************************************************************************/
static void task1(void);
static void task2(void);
static void task3(void);


/*******************************************************************************************************
 *                         PUBLIC FUNCTIONS DEFINITION
 ******************************************************************************************************/
int main(void) {
	/* Configure priority of SysTick and PendSV */
	NVIC_SetPriority(PendSV_IRQn, 0xFFFFFFFF);
	uint32_t realPriority = NVIC_GetPriority(PendSV_IRQn);
	NVIC_SetPriority(SysTick_IRQn, realPriority-1ul);

	/* Configure SysTick clock to generate tick every 1 ms */
	SystemCoreClockUpdate();
	SysTick_Config(SystemCoreClock/1000ul);
	__enable_irq();

	/* Start OS */
	OS_Init(NULL);

	OS_TaskCreate(&task1TCB,
			task1,
			1ul,
			(uint8_t *)"task1",
			task1Stack,
			256ul,
			NULL);

	OS_TaskCreate(&task2TCB,
			task2,
			2ul,
			(uint8_t *)"task2",
			task2Stack,
			256ul,
			NULL);

	OS_TaskCreate(&task3TCB,
			task3,
			3ul,
			(uint8_t *)"task3",
			task3Stack,
			256ul,
			NULL);

	OS_MutexInit(&mutex1, 0, NULL);

	OS_Start(NULL);

	/* This line should not be reached if OS is initialized properly */
	for(;;);

	return 0;
}



/******************************************** ***********************************************************
 *                         PRIVATE FUNCTIONS DEFINITION
 ******************************************************************************************************/
static void task1(void) {
	uint32_t t1 = 0ul;
	OS_MutexError_E errLocal = OS_MUTEX_ERROR_NONE;

	OS_delayTicks(2);
	while(1) {
		t1++;

		OS_MutexPend(&mutex1, 10, &errLocal);
		if (errLocal == OS_MUTEX_ERROR_NONE) {

			trace_printf("%d\n", cnt);
			OS_MutexPost(&mutex1, NULL);
			OS_delayTicks(1ul);

		} else {
			trace_puts("mutex timeout task1");
		}
	}
}


static void task2(void) {
	uint32_t t2 = 0ul;
	OS_MutexError_E errLocal = OS_MUTEX_ERROR_NONE;

	OS_delayTicks(2);
	while(2) {
		t2++;

		OS_MutexPend(&mutex1, 10, &errLocal);
		if (errLocal == OS_MUTEX_ERROR_NONE) {

			cnt++;
			OS_delayTicks(20);
			OS_MutexPost(&mutex1, NULL);

		} else {
			trace_puts("mutex timeout task2");
		}
	}
}


static void task3(void) {
	uint32_t t3 = 0ul;
	OS_MutexError_E errLocal = OS_MUTEX_ERROR_NONE;

	while(3) {
		t3++;

		OS_MutexPend(&mutex1, 10, &errLocal);
		if (errLocal == OS_MUTEX_ERROR_NONE) {
			while(OS_getOSTickCounter() != 4);
			OS_MutexPost(&mutex1, NULL);

		} else {
			trace_puts("mutex timeout task3");
		}
	}
}


/*******************************************************************************************************
 *                         INCLUDE FILES
 ******************************************************************************************************/
#include "main.h"

/*******************************************************************************************************
 *                         PRIVATE DEFINES
 ******************************************************************************************************/

/*******************************************************************************************************
 *                         PRIVATE DATA TYPES
 ******************************************************************************************************/

/*******************************************************************************************************
 *                         PRIVATE VARIABLES
 ******************************************************************************************************/
static uint32_t task1Stack[256ul], task2Stack[256ul];
static OS_TCB_S task1TCB, task2TCB;

OS_Mutex_T mutex1;

uint32_t cnt;


/*******************************************************************************************************
 *                         GLOBAL VARIABLES DEFINITION
 ******************************************************************************************************/

/*******************************************************************************************************
 *                         PRIVATE FUNCTIONS DECLARATION
 ******************************************************************************************************/
static void task1(void);
static void task2(void);


/*******************************************************************************************************
 *                         PUBLIC FUNCTIONS DEFINITION
 ******************************************************************************************************/
int main(void) {
	/* Configure priority of SysTick and PendSV */
	NVIC_SetPriority(PendSV_IRQn, 0xFFFFFFFF);
	uint32_t realPriority = NVIC_GetPriority(PendSV_IRQn);
	NVIC_SetPriority(SysTick_IRQn, realPriority-1ul);

	/* Configure SysTick clock to generate tick every 10 ms */
	SystemCoreClockUpdate();
	SysTick_Config(SystemCoreClock/100ul);
	__enable_irq();

	/* Start OS */
	OS_Init();

	OS_TaskCreate(&task1TCB,
			task1,
			0ul,
			(uint8_t *)"task1",
			task1Stack,
			256ul);

	OS_TaskCreate(&task2TCB,
			task2,
			1ul,
			(uint8_t *)"task2",
			task2Stack,
			256ul);

	OS_MutexInit(&mutex1);

	OS_Start();

	/* This line should not be reached if OS is initialized properly */
	for(;;);

	return 0;
}



/******************************************** ***********************************************************
 *                         PRIVATE FUNCTIONS DEFINITION
 ******************************************************************************************************/
static void task1(void) {
	uint32_t t1 = 0ul;

	while(1) {
		t1++;

		OS_MutexPend(&mutex1);
		cnt++;
		OS_delayTicks(2ul);
		OS_MutexPend(&mutex1);

	}
}


static void task2(void) {
	uint32_t t2 = 0ul;

	while(2) {
		t2++;

		OS_MutexPend(&mutex1);
		cnt++;
		OS_MutexPend(&mutex1);
	}
}


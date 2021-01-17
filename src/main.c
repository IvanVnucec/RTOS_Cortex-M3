/*******************************************************************************************************
 *                         INCLUDE FILES
 ******************************************************************************************************/
#include "stm32f1xx.h"

#include "bsp_led.h"
#include "os.h"
#include "mutex.h"


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
static uint32_t task4Stack[256ul];

static OS_TCB_S task1TCB;
static OS_TCB_S task2TCB;
static OS_TCB_S task3TCB;
static OS_TCB_S task4TCB;

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
static void task4(void);


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

	BSP_LED_Init();
	BSP_LED_On();

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

	OS_TaskCreate(&task4TCB,
		task4,
		4ul,
		(uint8_t *)"task4",
		task4Stack,
		256ul,
		NULL);

	OS_MutexInit(&mutex1, 5, NULL);

	/* enable global interurpts */
	__enable_irq();

	OS_EnableScheduler(NULL);

	/* This line should not be reached if OS is initialized properly */
	for(;;);

	return 0;
}


/******************************************** ***********************************************************
 *                         PRIVATE FUNCTIONS DEFINITION
 ******************************************************************************************************/
static void task1(void) {
	OS_MutexError_E errLocal = OS_MUTEX_ERROR_NONE;

	while(1) {
		OS_MutexPend(&mutex1, 20, &errLocal);
		if (errLocal == OS_MUTEX_ERROR_NONE) {
			OS_delayTicks(20);
			OS_MutexPost(&mutex1, NULL);

		} else {
			
		}
	}
}


static void task2(void) {
	OS_MutexError_E errLocal = OS_MUTEX_ERROR_NONE;	

	while(2) {
		OS_MutexPend(&mutex1, 20, &errLocal);
		if (errLocal == OS_MUTEX_ERROR_NONE) {
			OS_delayTicks(20);
			OS_MutexPost(&mutex1, NULL);

		} else {

		}
	}
}


static void task3(void) {
	OS_MutexError_E errLocal = OS_MUTEX_ERROR_NONE;

	OS_MutexPend(&mutex1, 1ul, &errLocal);
	if (errLocal == OS_MUTEX_ERROR_NONE) {
		OS_delayTicks(20ul);
		OS_MutexPost(&mutex1, NULL);

	} else {
		
	}
}


static void task4(void) {
	while(4) {
	    BSP_LED_Off();
        OS_delayTicks(OS_MS_TO_TICKS(500ul));
        BSP_LED_On();
        OS_delayTicks(OS_MS_TO_TICKS(500ul));
	}
}


/**
 * \file            main.c
 * \brief           main.c source file
 */


/*******************************************************************************************************
 *                         INCLUDE FILES
 ******************************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <libopencm3/cm3/nvic.h>

#include "bsp_led.h"
#include "os.h"
#include "mutex.h"

#include "clock.h"
#include "usart.h"


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

OS_Mutex_S mutex1;

uint32_t cnt;


/*******************************************************************************************************
 *                         GLOBAL VARIABLES DEFINITION
 ******************************************************************************************************/
extern uint32_t SystemCoreClock;


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


/**
  * @brief  		main function
  * @param 	    	None
  * @retval 		int
  */
int main(void) {
	// exit qemu with return value
	//exit(5);
	
	clock_setup();
    usart_setup();

	nvic_enable_irq(NVIC_SYSTICK_IRQ);
	nvic_set_priority(NVIC_SYSTICK_IRQ, 5ul);
	nvic_enable_irq(NVIC_PENDSV_IRQ);
	nvic_set_priority(NVIC_PENDSV_IRQ, 6ul);

	/* init and turn on status LED */
	BSP_LED_Init();
	BSP_LED_On();

	/* enable global interurpts */
	cm_enable_interrupts();

	/* Start OS */
	OS_Init(NULL);

	/* Create tasks */
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

	/* Call the scheduler */
	OS_EnableScheduler(NULL);

	/* This line should not be reached if OS is initialized properly */
	for(;;);

	return 0;
}


/******************************************** ***********************************************************
 *                         PRIVATE FUNCTIONS DEFINITION
 ******************************************************************************************************/

/**
  * @brief
  */
static void task1(void) {
	OS_MutexError_E errLocal = OS_MUTEX_ERROR_NONE;

	printf("Task1\n");

	while(1) {
		OS_MutexPend(&mutex1, 60, &errLocal);
		if (errLocal == OS_MUTEX_ERROR_NONE) {
			OS_delayTicks(20);
			OS_MutexPost(&mutex1, NULL);

		} else {
			
		}
	}
}


/**
  * @brief
  */
static void task2(void) {
	OS_MutexError_E errLocal = OS_MUTEX_ERROR_NONE;	

	printf("Task2\n");

	while(2) {
		OS_MutexPend(&mutex1, 40, &errLocal);
		if (errLocal == OS_MUTEX_ERROR_NONE) {
			OS_delayTicks(20);
			OS_MutexPost(&mutex1, NULL);

		} else {

		}
	}
}


/**
  * @brief
  */
static void task3(void) {
	OS_MutexError_E errLocal = OS_MUTEX_ERROR_NONE;

	printf("Task3\n");

	OS_MutexPend(&mutex1, 1ul, &errLocal);
	if (errLocal == OS_MUTEX_ERROR_NONE) {
		OS_delayTicks(20ul);
		OS_MutexPost(&mutex1, NULL);

	} else {
		
	}
}


/**
  * @brief
  */
static void task4(void) {
	while(4) {
		printf("Task4 %ld\n", OS_getOSTickCounter());
	    BSP_LED_Off();
        OS_delayTicks(OS_MS_TO_TICKS(500ul));
        BSP_LED_On();
        OS_delayTicks(OS_MS_TO_TICKS(500ul));
		exit(0);
	}
}

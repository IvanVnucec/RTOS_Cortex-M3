/**
 * \file            bsp_led.c
 * \brief           STM32F103 Blueplll Board Specific Package source file
 */


#include <stdint.h>

#include "stm32f1xx.h"
#include "bsp_led.h"


/**
  * @brief  		  Initializes PC13 LED on Bluepill board.
  * @param[in] 	  None
  * @retval 		  None
  */
void BSP_LED_Init(void) {
  /* Enable peripheral clock */
  SET_BIT(RCC->APB2ENR, RCC_APB2ENR_IOPCEN);

  /* Output mode, 2 MHz */
  CLEAR_BIT(GPIOC->CRH, GPIO_CRH_MODE13_1);
  SET_BIT(GPIOC->CRH, GPIO_CRH_MODE13_0);

  /* General purpose output, push-pull */
  CLEAR_BIT(GPIOC->CRH, GPIO_CRH_CNF13_0);
  CLEAR_BIT(GPIOC->CRH, GPIO_CRH_CNF13_1);
}


/**
  * @brief  		  Turn on PC13 LED.
  * @param[in] 	  None
  * @retval 		  None
  */
void BSP_LED_On(void) {
    GPIOC->BSRR = GPIO_BSRR_BR13_Msk;
}


/**
  * @brief  		  Turn off PC13 LED.
  * @param[in] 	  None
  * @retval 		  None
  */
void BSP_LED_Off(void) {
    GPIOC->BSRR = GPIO_BSRR_BS13_Msk;
}

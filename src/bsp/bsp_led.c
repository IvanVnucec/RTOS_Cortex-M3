#include <stdint.h>

#include "stm32f1xx.h"
#include "stm32f103xb.h"
#include "bsp_led.h"


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


void BSP_LED_On(void) {
    GPIOC->BSRR = GPIO_BSRR_BR13_Msk;
}


void BSP_LED_Off(void) {
    GPIOC->BSRR = GPIO_BSRR_BS13_Msk;
}

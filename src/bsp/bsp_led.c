/**
 * \file            bsp_led.c
 * \brief           STM32F103 Blueplll Board Specific Package source file
 */


#include <stdint.h>

#include "bsp_led.h"
#include "libopencm3/stm32/gpio.h"
#include <libopencm3/stm32/rcc.h>


/**
  * @brief  		  Initializes PC13 LED on Bluepill board.
  * @param[in] 	  None
  * @retval 		  None
  */
void BSP_LED_Init(void) {
    rcc_periph_clock_enable(RCC_GPIOC);

    gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO13);

}


/**
  * @brief  		  Turn on PC13 LED.
  * @param[in] 	  None
  * @retval 		  None
  */
void BSP_LED_On(void) {
    gpio_set(GPIOC, GPIO13);
}


/**
  * @brief  		  Turn off PC13 LED.
  * @param[in] 	  None
  * @retval 		  None
  */
void BSP_LED_Off(void) {
    gpio_clear(GPIOC, GPIO13);
}

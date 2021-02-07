#include "clock.h"
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/cm3/systick.h>


void clock_setup(void) {
    /* 24 MHz from internal crystal */
    rcc_clock_setup_pll(&rcc_hsi_configs[RCC_CLOCK_HSI_24MHZ]);

    /* 24 MHz / 8 => 4 MHz */
    systick_set_clocksource(STK_CSR_CLKSOURCE_AHB_DIV8);

    /* 4Mhz / 4000 = 1000 overflows per second - every 1ms one interrupt */
	/* SysTick interrupt every N clock pulses: set reload to N-1 */
	systick_set_reload(4000ul - 1ul);

    systick_interrupt_enable();

    /* Start counting. */
	systick_counter_enable();

}
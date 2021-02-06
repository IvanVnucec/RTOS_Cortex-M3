#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>

#include "usart.h"


void usart_setup(void) {
    // Clock for GPIO port A: GPIO_USART1_TX, USART1
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_USART1);

    // GPIO_USART1_TX/GPIO13 on GPIO port A for tx
	gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_USART1_TX);

	usart_set_baudrate(USART1, 115200);
	usart_set_databits(USART1, 8);
	usart_set_stopbits(USART1, USART_STOPBITS_1);
	usart_set_mode(USART1, USART_MODE_TX);
	usart_set_parity(USART1, USART_PARITY_NONE);
	usart_set_flow_control(USART1, USART_FLOWCONTROL_NONE);
	usart_enable(USART1);
}


void usart_teardown(void) {
    usart_disable(USART1);

}


int usart_putc(char c) {
    usart_send_blocking(USART1, c);

    return 0;
}


char usart_getc(void) {
    // Blocks until a character is captured from the UART
    uint16_t cr = usart_recv_blocking(USART1);

    return (char)cr;
}

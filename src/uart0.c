void UART0_putc(char c) {
    volatile unsigned int * const UART0 = ((volatile unsigned int*)0x4000C000);

    *UART0 = c;
}
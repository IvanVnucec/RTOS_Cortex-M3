/* This functions are used for interfacing serial port on QEMU emulator */

.cpu cortex-m3
.thumb

.equ    UART0, 0x4000C000

.thumb_func
.globl UART0_putc
UART0_putc:
    LDR     R1, =UART0
    STR     R0, [R1]
    BX      LR


.thumb_func
.globl UART0_getc
UART0_getc:
    LDR     R1, =UART0
    LDR     R0, [R1]
    BX      LR

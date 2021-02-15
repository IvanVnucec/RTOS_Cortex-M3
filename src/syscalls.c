#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

// LIBC SYSCALLS
/////////////////////

/* in uart0.s */
extern void UART0_putc(char c);
extern uint32_t UART0_getc(void);

extern int end;

caddr_t _sbrk(int incr) {
    static unsigned char *heap = NULL;
    unsigned char *prev_heap;

    if (heap == NULL) {
        heap = (unsigned char *)&end;
    }
    prev_heap = heap;

    heap += incr;

    return (caddr_t)prev_heap;
}

int _close(int file) {
    return -1;
}

int _fstat(int file, struct stat *st) {
    st->st_mode = S_IFCHR;

    return 0;
}

int _isatty(int file) {
    return 1;
}

int _lseek(int file, int ptr, int dir) {
    return 0;
}

void _exit(int status) {
    #ifdef QEMU_ENABLED
    printf("%d\n", status);

    register uint32_t r0 __asm__("r0") = 0x18;
    register uint32_t r1 __asm__("r1") = 0x20026;

    __asm__ volatile("bkpt #0xAB");

    (void)r0;
    (void)r1;
    #endif

    while(1);
}

void _kill(int pid, int sig) {
    return;
}

int _getpid(void) {
    return -1;
}

/* in order to print with printf functions on QEMU serial port
printf string must end with '\n' character.
*/
int _write(int file, char *ptr, int len) {
    int i;

    if (file == STDOUT_FILENO || file == STDERR_FILENO) {
        for (i = 0; i < len; i++) {
            if (ptr[i] == '\n') {
#ifdef QEMU_ENABLED
                UART0_putc('\r');
#endif
            }
#ifdef QEMU_ENABLED
            UART0_putc(ptr[i]);
#endif
        }
        return i;
    }
    errno = EIO;
    return -1;
}

int _read(int file, char *ptr, int len) {
    return -1;
}

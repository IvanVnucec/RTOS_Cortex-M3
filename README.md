# Cortex-M3 RTOS
 - Cortex-M3 RTOS, preemptive, priority-based, staticly linked

## Description
 - Scheduler is implemented in `os.c` file. Context switching is written in assembly language in `os_cpu.s` file. Simple example is written in `main.c` file.  
 
## Build
- To build it, use ARM GCC toolchain with GCC compiler.
- Best way to run it is to open a project with Eclipse.

## Testing
- We are testing it with STLINK-V2 OCD debugger on STM32 Bluepill board. 

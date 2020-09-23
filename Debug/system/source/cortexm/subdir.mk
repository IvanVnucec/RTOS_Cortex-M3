################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../system/source/cortexm/_initialize_hardware.c \
../system/source/cortexm/_reset_hardware.c \
../system/source/cortexm/exception_handlers.c 

OBJS += \
./system/source/cortexm/_initialize_hardware.o \
./system/source/cortexm/_reset_hardware.o \
./system/source/cortexm/exception_handlers.o 

C_DEPS += \
./system/source/cortexm/_initialize_hardware.d \
./system/source/cortexm/_reset_hardware.d \
./system/source/cortexm/exception_handlers.d 


# Each subdirectory must supply rules for building sources it contributes
system/source/cortexm/%.o: ../system/source/cortexm/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM GNU C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -ffreestanding -fno-move-loop-invariants -Wunused -Wuninitialized -Wall -Wextra -Wmissing-declarations -Wconversion -Wpointer-arith -Wpadded -Wshadow -Wlogical-op -Waggregate-return -Wfloat-equal  -g3 -DDEBUG -DUSE_FULL_ASSERT -DTRACE -DOS_USE_TRACE_SEMIHOSTING_DEBUG -DSTM32F10X_MD -DUSE_STDPERIPH_DRIVER -DHSE_VALUE=8000000 -I"../include" -I"../system/include" -I"../system/include/cmsis" -I"../system/include/stm32f1-stdperiph" -std=gnu11 -Wmissing-prototypes -Wstrict-prototypes -Wbad-function-cast -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



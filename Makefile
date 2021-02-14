PROJECT = rtos
BUILD_DIR = build
Q ?= @

CC = arm-none-eabi-gcc
LD = arm-none-eabi-ld
OCPY = arm-none-eabi-objcopy
MKDIR = mkdir
GIT=git
ECHO=@echo
CAT=cat
PYTHON ?= python
STFLASH	= $(shell which st-flash)



SRCS_APP = \
	src/clock.c \
	src/main.c \
	src/syscalls.c \
	src/usart.c \
	src/bsp/bsp_led.c \
	src/rtos/mutex.c \
	src/rtos/os_cpu.s \
	src/rtos/os.c

INCLUDES = \
	src \
	src/rtos \
	src/bsp


DEFINES += \
	STM32F1 \
	DSTM32F103xB

CFLAGS += \
  -mcpu=cortex-m3 \
  -mthumb \
  -Wall \
  -Werror \
  -std=gnu11 \
  -O0 \
  -g \
  -ffunction-sections \
  -fdata-sections

LDFLAGS += \
  -static \
  -nostartfiles \
  -specs=nano.specs \
  -Wl,--start-group -lc -lgcc -lnosys -Wl,--end-group \
  -Wl,-Map=$(BUILD_DIR)/$(PROJECT).map \

LDSCRIPT = stm32f103c8t6.ld
LDSCRIPT_QEMU = stm32f103c8t6_qemu.ld

LDFLAGS_APP = $(LDFLAGS) -T $(LDSCRIPT)
LDFLAGS_APP_QEMU = $(LDFLAGS) -T $(LDSCRIPT_QEMU)

OPENCM3_PATH = ./libopencm3
OPENCM3_INCLUDES = $(OPENCM3_PATH)/include
OPENCM3_LIB = $(OPENCM3_PATH)/lib/libopencm3_stm32f1.a

INCLUDES += $(OPENCM3_INCLUDES)
CFLAGS += $(foreach i,$(INCLUDES),-I$(i))
CFLAGS += $(foreach d,$(DEFINES),-D$(d))

DEFINES_QEMU = $(DEFINES) QEMU_ENABLED
INCLUDES_QEMU = $(OPENCM3_INCLUDES)
CFLAGS_QEMU = $(CFLAGS) $(foreach i,$(INCLUDES_QEMU),-I$(i))
CFLAGS_QEMU = $(CFLAGS) $(foreach d,$(DEFINES_QEMU),-D$(d))
SRCS_APP_QEMU = $(SRCS_APP) src/uart0.s

.PHONY: all
all: $(BUILD_DIR)/$(PROJECT).elf $(BUILD_DIR)/$(PROJECT).bin $(BUILD_DIR)/$(PROJECT)_qemu.elf $(BUILD_DIR)/$(PROJECT)_qemu.bin

$(BUILD_DIR)/$(PROJECT).elf: $(SRCS_APP) $(OPENCM3_LIB) Makefile
	$(ECHO) "  LD		$@"
	$(Q)$(MKDIR) -p $(BUILD_DIR)
	$(Q)$(CC) $(CFLAGS) $(LDFLAGS_APP) $(SRCS_APP) $(OPENCM3_LIB) -o $@

$(BUILD_DIR)/$(PROJECT)_qemu.elf: $(SRCS_APP_QEMU) $(OPENCM3_LIB) Makefile
	$(ECHO) "  LD		$@"
	$(Q)$(MKDIR) -p $(BUILD_DIR)
	$(Q)$(CC) $(CFLAGS_QEMU) $(LDFLAGS_APP_QEMU) $(SRCS_APP_QEMU) $(OPENCM3_LIB) -o $@

$(OPENCM3_LIB):
	$(ECHO) "Building libopencm3"
	$(Q)$(MAKE) -s -C $(OPENCM3_PATH) TARGETS=stm32/f1

$(BUILD_DIR)/$(PROJECT).bin: $(BUILD_DIR)/$(PROJECT).elf
	$(Q)$(OCPY) -Obinary $< $@

$(BUILD_DIR)/$(PROJECT)_qemu.bin: $(BUILD_DIR)/$(PROJECT)_qemu.elf
	$(Q)$(OCPY) -Obinary $< $@


.PHONY: qemu
qemu: $(BUILD_DIR)/$(PROJECT)_qemu.elf
	qemu-system-arm \
	-cpu cortex-m3 \
	-machine lm3s6965evb \
	-nographic \
	-semihosting-config enable=on,target=native \
	-gdb tcp::3333 \
	-S \
	-kernel $<


.PHONY: clean
clean:
	$(ECHO) "  CLEAN		rm -rf $(BUILD_DIR)"
	$(Q)rm -rf $(BUILD_DIR)


# Flash 64k Device
.PHONY: flash
flash: $(BUILD_DIR)/$(PROJECT).bin
	$(STFLASH) $(FLASHSIZE) write $< 0x8000000

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

GIT_SHA := \"$(shell $(GIT) rev-parse --short HEAD)\"


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

RENODE_REPO = renode

DEFINES += \
	STM32F1 \
	DSTM32F103xB \
	GIT_SHA=$(GIT_SHA) \

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

LDFLAGS_APP = $(LDFLAGS) -T $(LDSCRIPT)

OPENCM3_PATH = ./libopencm3
OPENCM3_INCLUDES = $(OPENCM3_PATH)/include
OPENCM3_LIB = $(OPENCM3_PATH)/lib/libopencm3_stm32f1.a

INCLUDES += $(OPENCM3_INCLUDES)
CFLAGS += $(foreach i,$(INCLUDES),-I$(i))
CFLAGS += $(foreach d,$(DEFINES),-D$(d))

.PHONY: all test_docker test_local renode
all: $(BUILD_DIR)/$(PROJECT).elf $(BUILD_DIR)/$(PROJECT).bin

$(BUILD_DIR)/$(PROJECT).elf: $(SRCS_APP) $(OPENCM3_LIB)
	$(ECHO) "  LD		$@"
	$(Q)$(MKDIR) -p $(BUILD_DIR)
	$(Q)$(CC) $(CFLAGS) $(LDFLAGS_APP) $^ -o $@

$(RENODE_REPO):
	$(ECHO) "renode not found, cloning it..."
	$(Q)$(GIT) clone https://github.com/renode/renode.git 2>1

$(OPENCM3_LIB):
	$(ECHO) "Building libopencm3"
	$(Q)$(MAKE) -s -C $(OPENCM3_PATH) TARGETS=stm32/f1

$(BUILD_DIR)/$(PROJECT).bin: $(BUILD_DIR)/$(PROJECT).elf
	$(OCPY) -Obinary $< $@

test_docker:
	./docker-test.sh

test_local: $(RENODE_REPO)
	./run_tests.sh

start_renode: $(BUILD_DIR)/$(PROJECT).elf
	./start.sh

.PHONY: clean
clean:
	$(ECHO) "  CLEAN		rm -rf $(BUILD_DIR)"
	$(Q)rm -rf $(BUILD_DIR)
	$(Q)make -C $(OPENCM3_PATH) TARGETS=stm32/f1 clean


# Flash 64k Device
.PHONY: flash
flash: $(BUILD_DIR)/$(PROJECT).bin
	$(STFLASH) $(FLASHSIZE) write $< 0x8000000

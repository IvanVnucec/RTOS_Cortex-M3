# ------------------------------------------------
# Generic Makefile (based on gcc)
# ------------------------------------------------

######################################
# target
######################################
TARGET = all_tests


######################################
# building variables
######################################
# debug build?
DEBUG = 1
# optimization
OPT = -Og -O0


#######################################
# paths
#######################################
# Build path
BUILD_DIR = build

######################################
# source
######################################
# C sources
C_SOURCES =  \
../src/rtos/mutex.c \
src/test_mutex.c

# ASM sources
ASM_SOURCES = 


#######################################
# binaries
#######################################
PREFIX = 
# The gcc compiler bin path can be either defined in make command via GCC_PATH variable (> make GCC_PATH=xxx)
# either it can be added to the PATH environment variable.
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S
 
#######################################
# CFLAGS
#######################################

# macros for gcc
# AS defines
AS_DEFS =

# C defines
C_DEFS =  

# AS includes
AS_INCLUDES =

# C includes
C_INCLUDES =  \
-I./header_overrides \
-I./../src/rtos \
-Isrc \
-Imocks \
-Ifakes


# compile gcc flags
ASFLAGS = $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -Wextra

CFLAGS = $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -Wextra

ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
endif

# Generate dependency information
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"

# default action: build all
all: $(BUILD_DIR)/$(TARGET)


#######################################
# build the application
#######################################
# list of objects
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
# list of ASM program objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR) 
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET): $(OBJECTS) Makefile
	$(CC) $(OBJECTS) -o $@
	$(SZ) $@
	
$(BUILD_DIR):
	mkdir $@		


#######################################
# clean up
#######################################
.PHONY: clean
clean:
	-rm -fR $(BUILD_DIR)


#######################################
# run test
#######################################
.PHONY: test
test: $(BUILD_DIR)/$(TARGET)
	-$<
  

#######################################
# dependencies
#######################################
-include $(wildcard $(BUILD_DIR)/*.d)

# *** EOF ***

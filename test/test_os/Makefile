# TEST MAKEFILE

TARGET = test_os

BUILD_DIR = ./build
SRC_DIR = ./../../src
RTOS_DIR = $(SRC_DIR)/rtos
HEADER_OVERRIDES_DIR = ./header_overrides
MINUNIT_DIR = ./../

C_SOURCES =  \
test_os.c

C_INCLUDES =  \
-I$(MINUNIT_DIR)

C_DEFS = \
-DUNIT_TESTING

OPT = -O0

CC = gcc

CFLAGS = $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -Wextra

CFLAGS += -g

CFLAGS += --coverage
LDFLAGS += --coverage

# Generate dependency information
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"


# default action: build all
all: $(BUILD_DIR)/$(TARGET)


# list of objects
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))


$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR) 
	$(CC) $(LDFLAGS) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@


$(BUILD_DIR)/$(TARGET): $(OBJECTS) Makefile
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@


$(BUILD_DIR):
	mkdir $@


.PHONY: clean
clean:
	-rm -fR $(BUILD_DIR)


.PHONY: run
run: $(BUILD_DIR)/$(TARGET)
	./$<


-include $(wildcard $(BUILD_DIR)/*.d)

# Binaries will be generated with this name (.elf, .bin, .hex, etc)
PROJ_NAME=GCC_STM32F429I-Discovery

# Put your source files here (or *.c, etc)
SRCS=src/main.c src/system_stm32f4xx.c src/startup_stm32f429x.S SPL/src/*.c ../../source/*.c ../../source/port/GCC/ARM_Cortex_M4/*.c
INCS=-Iinc -Icmsis -Iconfig -ISPL/inc -I../../include -I../../include/port/GCC/ARM_Cortex_M4

# Normally you shouldn't need to change anything below this line!
#######################################################################################

PREFIX=arm-none-eabi-
CC=$(PREFIX)gcc
OBJCOPY=$(PREFIX)objcopy

CFLAGS  = -g -O2 -Wall -Tstm32f429zi_flash.ld 
CFLAGS += -mlittle-endian -mthumb -mcpu=cortex-m4 -mthumb-interwork
CFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16

OBJS = $(SRCS:.c=.o)

.PHONY: all

all: $(PROJ_NAME).elf

$(PROJ_NAME).elf: $(SRCS)
	$(CC) $(CFLAGS) $(INCS) $^ -o $@ 
	$(OBJCOPY) -O ihex $(PROJ_NAME).elf $(PROJ_NAME).hex
	$(OBJCOPY) -O binary $(PROJ_NAME).elf $(PROJ_NAME).bin

clean:
	rm -f *.o $(PROJ_NAME).elf $(PROJ_NAME).hex $(PROJ_NAME).bin


# put your *.o targets here, make should handle the rest!

SRCS = 	apps/src/main.c \
		common/src/io.c \
		common/src/mem.c \
		common/src/string.c \
		common/src/thlist.c \
		core/src/dbgterm.c \
		core/src/device.c \
		core/src/event.c \
		core/src/heap.c \
		core/src/intrvect.c \
		core/src/lock.c \
		core/src/queue.c \
		core/src/rtc.c \
		core/src/system.c \
		core/src/terminal.c \
		core/src/thread.c \
		core/src/timer.c \
		drivers/src/i2c.c \
		drivers/src/serial.c \
		drivers/src/spi.c \
		hw/boards/src/stm32f4discovery.c \
		hw/compilers/gcc/src/plat_comp_dep.c \
		hw/platform/src/plat_cpu.c \
		hw/platform/src/plat_i2c.c \
		hw/platform/src/plat_rtc.c \
		hw/platform/src/plat_spi.c \
		hw/platform/src/plat_uart.c \
		hw/st/src/system_stm32f4xx.c \
		
# all the files will be generated with this name (main.elf, main.bin, main.hex, etc)

PROJ_NAME=main

# Put your stlink folder here so make burn will work.

STLINK=/mnt/share/Programming/embedded/stm32/stlink

# that's it, no need to change anything below this line!

###################################################

CC=arm-none-eabi-gcc
OBJCOPY=arm-none-eabi-objcopy

CFLAGS  = -g -Wall -Thw/compilers/stm32f4xx_flash.ld 
CFLAGS += -mlittle-endian -mthumb -mcpu=cortex-m4 -mthumb-interwork
CFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16

###################################################

vpath %.c src
vpath %.a lib

ROOT=$(shell pwd)

CFLAGS += -I. 
CFLAGS += -Iapps/inc
CFLAGS += -Icommon/inc
CFLAGS += -Icore/inc
CFLAGS += -Idrivers/inc
CFLAGS += -Ihw/boards/inc
CFLAGS += -Ihw/compilers/gcc/inc
CFLAGS += -Ihw/platform/inc
CFLAGS += -Ihw/st/inc
CFLAGS += -Ihw/st/inc/core
CFLAGS += -Ihw/st/inc/peripherals
CFLAGS += -Iutils/inc

SRCS += hw/compilers/gcc/startup_stm32f4xx.s # add startup file to build

OBJS = $(SRCS:.c=.o)

###################################################

.PHONY: lib proj

all: lib proj

again: clean all

# Flash the STM32F4
burn:
	$(STLINK)/flash/st-flash write $(PROJ_NAME).bin 0x8000000

# Create tags; assumes ctags exists
ctags:
	ctags -R --exclude=*cm0.h --exclude=*cm3.h .

lib:
	$(MAKE) -C hw/st

proj: 	$(PROJ_NAME).elf

$(PROJ_NAME).elf: $(SRCS)
	$(CC) $(CFLAGS) $^ -o $@ -Lhw/st -lstm32f4
	$(OBJCOPY) -O ihex $(PROJ_NAME).elf $(PROJ_NAME).hex
	$(OBJCOPY) -O binary $(PROJ_NAME).elf $(PROJ_NAME).bin

clean:
	rm -f *.o *.i
	rm -f $(PROJ_NAME).elf
	rm -f $(PROJ_NAME).hex
	rm -f $(PROJ_NAME).bin

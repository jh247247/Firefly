CC=arm-none-eabi-gcc
AR=arm-none-eabi-ar

###########################################

vpath %.c .

STD_PERIPH_LIB=../StdPeriph

CFLAGS  = -g -Os -Wall
CFLAGS += -mlittle-endian -mthumb -mcpu=cortex-m0 -march=armv6s-m
CFLAGS += -ffreestanding -nostdlib
CFLAGS += -I $(STD_PERIPH_LIB) -I $(STD_PERIPH_LIB)/CMSIS/Device/ST/STM32F0xx/Include
CFLAGS += -I $(STD_PERIPH_LIB)/CMSIS/Include -I $(STD_PERIPH_LIB)/STM32F0xx_StdPeriph_Driver/inc

SRCS = jio.c

OBJS = $(SRCS:.c=.o)

.PHONY: libjio.a

all: libjio.a

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $^

libjio.a: $(OBJS)
	$(AR) -r $@ $(OBJS)

clean:
	rm -f $(OBJS) libjio.a

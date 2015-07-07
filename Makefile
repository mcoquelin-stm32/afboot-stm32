CROSS_COMPILE ?= arm-none-eabi-

CC = $(CROSS_COMPILE)gcc
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump
SIZE = $(CROSS_COMPILE)size
GDB = $(CROSS_COMPILE)gdb
OPENOCD = openocd

CFLAGS := -mthumb -mcpu=cortex-m4
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += -Os -std=gnu99 -Wall
LDFLAGS := -nostartfiles -Wl,--gc-sections

obj-y += stm32f429i-disco.o

all: stm32f429i-disco.elf test

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

stm32f429i-disco.elf: $(obj-y) Makefile
	$(CC) -T stm32f429.lds $(LDFLAGS) -o stm32f429i-disco.elf $(obj-y)

test: stm32f429i-disco.elf Makefile
	$(OBJCOPY) -Obinary stm32f429i-disco.elf stm32f429i-disco.bin
	$(OBJDUMP) -S stm32f429i-disco.elf > stm32f429i-disco.lst
	$(SIZE) stm32f429i-disco.elf

clean:
	@rm -f *.o *.elf *.bin *.lst

flash: test
	$(OPENOCD) -f board/stm32f429discovery.cfg \
	  -c "init" \
	  -c "reset init" \
	  -c "flash probe 0" \
	  -c "flash info 0" \
	  -c "flash write_image erase stm32f429i-disco.bin 0x08000000" \
	  -c "reset run" \
	  -c "shutdown"

debug: test
	$(GDB) stm32f429i-disco.elf -ex "target remote :3333" -ex "monitor reset halt"

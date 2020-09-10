CROSS_COMPILE ?= arm-none-eabi-

CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump
SIZE = $(CROSS_COMPILE)size
GDB = $(CROSS_COMPILE)gdb
OPENOCD = openocd
KERNEL_ADDR?=0x08008000
DTB_ADDR?=0x08004000

CFLAGS := -mthumb -mcpu=cortex-m4
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += -Os -std=gnu99 -Wall
CFLAGS += -fno-builtin
LINKERFLAGS := -nostartfiles --gc-sections

obj-y += gpio.o mpu.o qspi.o start_kernel.o
obj-f4 += $(obj-y) usart-f4.o
obj-f7 += $(obj-y) usart-f7.o

all: stm32f429i-disco stm32429i-eval stm32f469i-disco stm32746g-eval stm32h743i-eval

%.o: %.c
	$(CC) -c $(CFLAGS) -DKERNEL_ADDR=$(KERNEL_ADDR) -DDTB_ADDR=$(DTB_ADDR) $< -o $@

stm32f429i-disco: stm32f429i-disco.o $(obj-f4)
	$(LD) -T stm32f429.lds $(LINKERFLAGS) -o stm32f429i-disco.elf stm32f429i-disco.o $(obj-f4)
	$(OBJCOPY) -Obinary stm32f429i-disco.elf stm32f429i-disco.bin
	$(SIZE) stm32f429i-disco.elf

stm32429i-eval: stm32429i-eval.o $(obj-f4)
	$(LD) -T stm32f429.lds $(LINKERFLAGS) -o stm32429i-eval.elf stm32429i-eval.o $(obj-f4)
	$(OBJCOPY) -Obinary stm32429i-eval.elf stm32429i-eval.bin
	$(SIZE) stm32429i-eval.elf

stm32f469i-disco: stm32f469i-disco.o $(obj-f4)
	$(LD) -T stm32f429.lds $(LINKERFLAGS) -o stm32f469i-disco.elf stm32f469i-disco.o $(obj-f4)
	$(OBJCOPY) -Obinary stm32f469i-disco.elf stm32f469i-disco.bin
	$(SIZE) stm32f469i-disco.elf

stm32746g-eval: stm32746g-eval.o $(obj-f7)
	$(LD) -T stm32f429.lds $(LINKERFLAGS) -o stm32746g-eval.elf stm32746g-eval.o $(obj-f7)
	$(OBJCOPY) -Obinary stm32746g-eval.elf stm32746g-eval.bin
	$(SIZE) stm32746g-eval.elf

stm32h743i-eval: stm32h743i-eval.o $(obj-f7)
	$(LD) -T stm32h743.lds $(LINKERFLAGS) -o stm32h743i-eval.elf stm32h743i-eval.o $(obj-f7)
	$(OBJCOPY) -Obinary stm32h743i-eval.elf stm32h743i-eval.bin
	$(SIZE) stm32h743i-eval.elf

clean:
	@rm -f *.o *.elf *.bin *.lst

flash_stm32f429i-disco: stm32f429i-disco
	$(OPENOCD) -f board/stm32f429discovery.cfg \
	  -c "init" \
	  -c "reset init" \
	  -c "flash probe 0" \
	  -c "flash info 0" \
	  -c "flash write_image erase stm32f429i-disco.bin 0x08000000" \
	  -c "reset run" \
	  -c "shutdown"

flash_stm32429i-eval: stm32429i-eval
	$(OPENOCD) -f board/stm32429i_eval_stlink.cfg \
	  -c "init" \
	  -c "reset init" \
	  -c "flash probe 0" \
	  -c "flash info 0" \
	  -c "flash write_image erase stm32429i-eval.bin 0x08000000" \
	  -c "reset run" \
	  -c "shutdown"

flash_stm32f469i-disco: stm32f469i-disco
	$(OPENOCD) -f board/stm32f469discovery.cfg \
	  -c "init" \
	  -c "reset init" \
	  -c "flash probe 0" \
	  -c "flash info 0" \
	  -c "flash write_image erase stm32f469i-disco.bin 0x08000000" \
	  -c "reset run" \
	  -c "shutdown"

flash_stm32746g-eval: stm32746g-eval
	$(OPENOCD) -f interface/stlink-v2-1.cfg -f board/stm327x6g_eval.cfg \
	  -c "init" \
	  -c "reset init" \
	  -c "flash probe 0" \
	  -c "flash info 0" \
	  -c "flash write_image erase stm32746g-eval.bin 0x08000000" \
	  -c "reset run" \
	  -c "shutdown"

flash_stm32h743i-eval: stm32h743i-eval
	$(OPENOCD) -f interface/stlink-v2-1.cfg -f board/stm32h7xx_eval.cfg \
	  -c "init" \
	  -c "reset init"\
	  -c "flash probe 0" \
	  -c "flash info 0" \
	  -c "flash write_image erase stm32h743i-eval.bin 0x08000000" \
	  -c "reset run" \
	  -c "shutdown"

debug_stm32f429i-disco: stm32f429i-disco
	$(GDB) stm32f429i-disco.elf -ex "target remote :3333" -ex "monitor reset halt"

debug_stm32429i-eval: stm32429i-eval
	$(GDB) stm32429i-eval.elf -ex "target remote :3333" -ex "monitor reset halt"

debug_stm32f469i-disco: stm32f469i-disco
	$(GDB) stm32f469i-disco.elf -ex "target remote :3333" -ex "monitor reset halt"

debug_stm32746g-eval: stm32746g-eval
	$(GDB) stm32746g-eval.elf -ex "target remote :3333" -ex "monitor reset halt"

debug_stm32h743i-eval: stm32h743i-eval
	$(GDB) stm32h743i-eval.elf -ex "target remote :3333" -ex "monitor reset halt"

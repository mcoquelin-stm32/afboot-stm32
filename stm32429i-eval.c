#include <stdlib.h>
#include <stdint.h>

#include "stm32f4_regs.h"
#include "usart.h"
#include "gpio.h"
#include "mpu.h"
#include "start_kernel.h"

#define CONFIG_HSE_HZ	25000000
#define CONFIG_PLL_M	25
#define CONFIG_PLL_N	360
#define CONFIG_PLL_P	2
#define CONFIG_PLL_Q	7
#define PLLCLK_HZ (((CONFIG_HSE_HZ / CONFIG_PLL_M) * CONFIG_PLL_N) / CONFIG_PLL_P)
#if PLLCLK_HZ == 180000000
#define FLASH_LATENCY	5
#else
#error PLL clock does not match 180 MHz
#endif

static void *usart_base = (void *)USART1_BASE;
static void *gpio_base = (void *)GPIOA_BASE;

static void clock_setup(void)
{
	volatile uint32_t *RCC_CR = (void *)(RCC_BASE + 0x00);
	volatile uint32_t *RCC_PLLCFGR = (void *)(RCC_BASE + 0x04);
	volatile uint32_t *RCC_CFGR = (void *)(RCC_BASE + 0x08);
	volatile uint32_t *FLASH_ACR = (void *)(FLASH_BASE + 0x00);
	volatile uint32_t *RCC_AHB1ENR = (void *)(RCC_BASE + 0x30);
	volatile uint32_t *RCC_AHB2ENR = (void *)(RCC_BASE + 0x34);
	volatile uint32_t *RCC_AHB3ENR = (void *)(RCC_BASE + 0x38);
	volatile uint32_t *RCC_APB1ENR = (void *)(RCC_BASE + 0x40);
	volatile uint32_t *RCC_APB2ENR = (void *)(RCC_BASE + 0x44);
	uint32_t val;

	*RCC_CR |= RCC_CR_HSEON;
	while (!(*RCC_CR & RCC_CR_HSERDY));

	val = *RCC_CFGR;
	val &= ~RCC_CFGR_HPRE_MASK;
	//val |= 0 << 4; // not divided
	val &= ~RCC_CFGR_PPRE1_MASK;
	val |= 0x5 << 10; // divided by 4
	val &= ~RCC_CFGR_PPRE2_MASK;
	val |= 0x4 << 13; // divided by 2
	*RCC_CFGR = val;

	val = 0;
	val |= RCC_PLLCFGR_PLLSRC_HSE;
	val |= CONFIG_PLL_M;
	val |= CONFIG_PLL_N << 6;
	val |= ((CONFIG_PLL_P >> 1) - 1) << 16;
	val |= CONFIG_PLL_Q << 24;
	*RCC_PLLCFGR = val;

	*RCC_CR |= RCC_CR_PLLON;
	while (*RCC_CR & RCC_CR_PLLRDY) {
	}

	*FLASH_ACR = FLASH_ACR_ICEN | FLASH_ACR_PRFTEN | FLASH_LATENCY;

	*RCC_CFGR &= ~RCC_CFGR_SW_MASK;
	*RCC_CFGR |= RCC_CFGR_SW_PLL;
	while ((*RCC_CFGR & RCC_CFGR_SWS_MASK) != RCC_CFGR_SWS_PLL) {
	}

	/*  Enable all clocks, unused ones will be gated at end of kernel boot */
	*RCC_AHB1ENR |= 0x7ef417ff;
	*RCC_AHB2ENR |= 0xf1;
	*RCC_AHB3ENR |= 0x1;
	*RCC_APB1ENR |= 0xf6fec9ff;
	*RCC_APB2ENR |= 0x4777f33;

}



static void fmc_wait_busy(void)
{
	volatile uint32_t *FMC_SDSR = (void *)(FMC_BASE + 0x158);

	while ((*FMC_SDSR & FMC_SDSR_BUSY)) {
	}
}

int main(void)
{
	volatile uint32_t *FLASH_KEYR = (void *)(FLASH_BASE + 0x04);
	volatile uint32_t *FLASH_CR = (void *)(FLASH_BASE + 0x10);
	volatile uint32_t *FMC_SDCR1 = (void *)(FMC_BASE + 0x140);
	volatile uint32_t *FMC_SDTR1 = (void *)(FMC_BASE + 0x148);
	volatile uint32_t *FMC_SDCMR = (void *)(FMC_BASE + 0x150);
	volatile uint32_t *FMC_SDRTR = (void *)(FMC_BASE + 0x154);
	volatile uint32_t *SYSCFG_MEMRMP = (void *)(SYSCFG_BASE + 0x00);
	int i;

	mpu_config(0x0);

	if (*FLASH_CR & FLASH_CR_LOCK) {
		*FLASH_KEYR = 0x45670123;
		*FLASH_KEYR = 0xCDEF89AB;
	}
	*FLASH_CR &= ~(FLASH_CR_ERRIE | FLASH_CR_EOPIE | FLASH_CR_PSIZE_MASK);
	*FLASH_CR |= FLASH_CR_PSIZE_X32;
	*FLASH_CR |= FLASH_CR_LOCK;

	clock_setup();

	gpio_set_fmc(gpio_base, 'D', 0); //D2
	gpio_set_fmc(gpio_base, 'D', 1); //D3
	gpio_set_fmc(gpio_base, 'D', 8); //D13
	gpio_set_fmc(gpio_base, 'D', 9); //D14
	gpio_set_fmc(gpio_base, 'D', 10); //D15
	gpio_set_fmc(gpio_base, 'D', 11); //A16
	gpio_set_fmc(gpio_base, 'D', 12); //A17
	gpio_set_fmc(gpio_base, 'D', 13); //A18
	gpio_set_fmc(gpio_base, 'D', 14); //D0
	gpio_set_fmc(gpio_base, 'D', 15); //D1
	gpio_set_fmc(gpio_base, 'E', 0); //NBL0
	gpio_set_fmc(gpio_base, 'E', 1); //NBL1
	gpio_set_fmc(gpio_base, 'E', 7); //D4
	gpio_set_fmc(gpio_base, 'E', 8); //D5
	gpio_set_fmc(gpio_base, 'E', 9); //D6
	gpio_set_fmc(gpio_base, 'E', 10); //D7
	gpio_set_fmc(gpio_base, 'E', 11); //D8
	gpio_set_fmc(gpio_base, 'E', 12); //D9
	gpio_set_fmc(gpio_base, 'E', 13); //D10
	gpio_set_fmc(gpio_base, 'E', 14); //D11
	gpio_set_fmc(gpio_base, 'E', 15); //D12
	gpio_set_fmc(gpio_base, 'F', 0); //A0
	gpio_set_fmc(gpio_base, 'F', 1); //A1
	gpio_set_fmc(gpio_base, 'F', 2); //A2
	gpio_set_fmc(gpio_base, 'F', 3); //A3
	gpio_set_fmc(gpio_base, 'F', 4); //A4
	gpio_set_fmc(gpio_base, 'F', 5); //A5
	gpio_set_fmc(gpio_base, 'F', 11); //SDNRAS
	gpio_set_fmc(gpio_base, 'F', 12); //A6
	gpio_set_fmc(gpio_base, 'F', 13); //A7
	gpio_set_fmc(gpio_base, 'F', 14); //A8
	gpio_set_fmc(gpio_base, 'F', 15); //A9
	gpio_set_fmc(gpio_base, 'G', 0); //A10
	gpio_set_fmc(gpio_base, 'G', 1); //A11
	gpio_set_fmc(gpio_base, 'G', 2); //A12
	gpio_set_fmc(gpio_base, 'G', 3); //A13
	gpio_set_fmc(gpio_base, 'G', 4); //A14
	gpio_set_fmc(gpio_base, 'G', 5); //A15
	gpio_set_fmc(gpio_base, 'G', 8); //SDCLK
	gpio_set_fmc(gpio_base, 'G', 15); //SDNCAS
	gpio_set_fmc(gpio_base, 'H', 2); //SDCKE0
	gpio_set_fmc(gpio_base, 'H', 3); //SDNE0
	gpio_set_fmc(gpio_base, 'H', 5); //SDNWE
	gpio_set_fmc(gpio_base, 'H', 8); //D16
	gpio_set_fmc(gpio_base, 'H', 9); //D17
	gpio_set_fmc(gpio_base, 'H', 10); //D18
	gpio_set_fmc(gpio_base, 'H', 11); //D19
	gpio_set_fmc(gpio_base, 'H', 12); //D20
	gpio_set_fmc(gpio_base, 'H', 13); //D21
	gpio_set_fmc(gpio_base, 'H', 14); //D22
	gpio_set_fmc(gpio_base, 'H', 15); //D23
	gpio_set_fmc(gpio_base, 'I', 0); //D24
	gpio_set_fmc(gpio_base, 'I', 1); //D25
	gpio_set_fmc(gpio_base, 'I', 2); //D26
	gpio_set_fmc(gpio_base, 'I', 3); //D27
	gpio_set_fmc(gpio_base, 'I', 4); //NBL2
	gpio_set_fmc(gpio_base, 'I', 5); //NBL3
	gpio_set_fmc(gpio_base, 'I', 6); //D28
	gpio_set_fmc(gpio_base, 'I', 7); //D29
	gpio_set_fmc(gpio_base, 'I', 9); //D30
	gpio_set_fmc(gpio_base, 'I', 10); //D31
	*FMC_SDCR1 = 0x000019E5;
	*FMC_SDTR1 = 0x01115351;

	fmc_wait_busy();
	*FMC_SDCMR = 0x00000011; // clock
	for (i = 0; i < 50000000; i++) { // 10 ms
		asm volatile ("nop");
	}
	fmc_wait_busy();
	*FMC_SDCMR = 0x00000012; // PALL
	fmc_wait_busy();
	*FMC_SDCMR = 0x00000073; // auto-refresh
	fmc_wait_busy();
	*FMC_SDCMR = 0x00046014; // external memory mode
	fmc_wait_busy();

	*FMC_SDRTR |= 2812<<1; // refresh rate
	*FMC_SDCR1 &= 0xFFFFFDFF;

	gpio_set_usart(gpio_base, 'A', 9, 7);
	gpio_set_usart(gpio_base, 'A', 10, 7);

	usart_setup(usart_base, 90000000);
	usart_putch(usart_base, '.');

	*SYSCFG_MEMRMP = 0x4;

	start_kernel();

	return 0;
}

static void noop(void)
{
	usart_putch(usart_base, 'E');
	while (1) {
	}
}

extern unsigned int _end_text;
extern unsigned int _start_data;
extern unsigned int _end_data;
extern unsigned int _start_bss;
extern unsigned int _end_bss;

void reset(void)
{
	unsigned int *src, *dst;

	asm volatile ("cpsid i");

	src = &_end_text;
	dst = &_start_data;
	while (dst < &_end_data) {
		*dst++ = *src++;
	}

	dst = &_start_bss;
	while (dst < &_end_bss) {
		*dst++ = 0;
	}

	main();
}

extern unsigned long _stack_top;

__attribute__((section(".vector_table")))
void (*vector_table[16 + 91])(void) = {
	(void (*))&_stack_top,
	reset,
	noop,
	noop,
	noop,
	noop,
	noop,
	NULL,
	NULL,
	NULL,
	NULL,
	noop,
	noop,
	NULL,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
	noop,
};


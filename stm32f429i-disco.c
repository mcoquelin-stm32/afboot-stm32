#include <stdlib.h>
#include <stdint.h>

#include "usart.h"

#define RCC_BASE	0x40023800

#define RCC_CR_HSEON	(1 << 16)
#define RCC_CR_HSERDY	(1 << 17)
#define RCC_CR_PLLON	(1 << 24)
#define RCC_CR_PLLRDY	(1 << 25)

#define RCC_PLLCFGR_PLLSRC_HSE	(1 << 22)

#define RCC_CFGR_SW_PLL		(0x2 << 0)
#define RCC_CFGR_SW_MASK	(0x3 << 0)
#define RCC_CFGR_SWS_PLL	(0x2 << 2)
#define RCC_CFGR_SWS_MASK	(0x3 << 2)
#define RCC_CFGR_HPRE_MASK	(0xf << 4)
#define RCC_CFGR_PPRE1_MASK	(0x7 << 10)
#define RCC_CFGR_PPRE2_MASK	(0x7 << 13)

#define RCC_AHB1ENR_GPIOGEN	(1 << 6)

#define RCC_AHB3ENR_FMC		(1 << 0)


#define RCC_APB2ENR_USART1EN	(1 << 4)
#define RCC_APB2ENR_SYSCFGEN	(1 << 14)

#define FLASH_BASE	0x40023C00

#define FLASH_ACR_PRFTEN	(1 << 8)
#define FLASH_ACR_ICEN		(1 << 9)

#define FLASH_CR_PSIZE_X32	0x2
#define FLASH_CR_PSIZE_MASK	0x3
#define FLASH_CR_EOPIE		(1 << 24)
#define FLASH_CR_ERRIE		(1 << 25)
#define FLASH_CR_LOCK		(1UL << 31)

#define CONFIG_HSE_HZ	8000000
#define CONFIG_PLL_M	8
#define CONFIG_PLL_N	360
#define CONFIG_PLL_P	2
#define CONFIG_PLL_Q	7
#define PLLCLK_HZ (((CONFIG_HSE_HZ / CONFIG_PLL_M) * CONFIG_PLL_N) / CONFIG_PLL_P)
#if PLLCLK_HZ == 180000000
#define FLASH_LATENCY	5
#else
#error PLL clock does not match 180 MHz
#endif

#define USART3_BASE	0x40004800
#define USART1_BASE	0x40011000

static void *usart_base = (void *)USART1_BASE;

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
	while (!(*RCC_CR & RCC_CR_HSERDY)) {
	}

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
}

#define GPIOA_BASE	0x40020000UL

#define GPIOx_MODER_MODERy_INPUT	0x0UL
#define GPIOx_MODER_MODERy_GPOUTPUT	0x1UL
#define GPIOx_MODER_MODERy_ALTFUNC	0x2UL
#define GPIOx_MODER_MODERy_ANALOG	0x3UL
#define GPIOx_MODER_MODERy_MASK		0x3UL

#define GPIOx_OSPEEDR_OSPEEDRy_FAST	0x2UL
#define GPIOx_OSPEEDR_OSPEEDRy_HIGH	0x3UL
#define GPIOx_OSPEEDR_OSPEEDRy_MASK	0x3UL

	/*  Enable all clocks, unused ones will be gated at end of kernel boot */
	*RCC_AHB1ENR |= 0x7ef417ff;
	*RCC_AHB2ENR |= 0xf1;
	*RCC_AHB3ENR |= 0x1;
	*RCC_APB1ENR |= 0xf6fec9ff;
	*RCC_APB2ENR |= 0x4777f33;
#define GPIOx_PUPDR_PUPDRy_MASK		0x3UL

#define GPIOx_AFRy_MASK	0xfUL

static void gpio_set(char bank, uint8_t port,
	uint8_t otype, uint8_t mode, uint8_t ospeed, uint8_t pupd)
{
	volatile uint32_t *RCC_AHB1ENR = (void *)(RCC_BASE + 0x30);
	volatile uint32_t *GPIOx_base = (void *)(GPIOA_BASE + (bank - 'A') * 0x400);
	volatile uint32_t *GPIOx_MODER   = (void *)GPIOx_base + 0x00;
	volatile uint32_t *GPIOx_OTYPER  = (void *)GPIOx_base + 0x04;
	volatile uint32_t *GPIOx_OSPEEDR = (void *)GPIOx_base + 0x08;
	volatile uint32_t *GPIOx_PUPDR   = (void *)GPIOx_base + 0x0C;
	int i;

	*RCC_AHB1ENR |= 1 << (bank - 'A');

	i = port;
	*GPIOx_OTYPER &= ~(1UL << i);
	*GPIOx_OTYPER |= (uint32_t)otype << i;

	i <<= 1;
	*GPIOx_MODER &= ~(GPIOx_MODER_MODERy_MASK << i);
	*GPIOx_MODER |= (uint32_t)mode << i;
	*GPIOx_OSPEEDR &= ~(GPIOx_OSPEEDR_OSPEEDRy_MASK << i);
	*GPIOx_OSPEEDR |= (uint32_t)ospeed << i;
	*GPIOx_PUPDR &= ~(GPIOx_PUPDR_PUPDRy_MASK << i);
	*GPIOx_PUPDR |= (uint32_t)pupd << i;
}

static void gpio_set_alt(char bank, uint8_t port,
	uint8_t otype, uint8_t ospeed, uint8_t pupd, uint8_t altfunc)
{
	volatile uint32_t *RCC_AHB1ENR = (void *)(RCC_BASE + 0x30);
	volatile uint32_t *GPIOx_base = (void *)(GPIOA_BASE + (bank - 'A') * 0x400);
	volatile uint32_t *GPIOx_AFRL = (void *)GPIOx_base + 0x20;
	volatile uint32_t *GPIOx_AFRH = (void *)GPIOx_base + 0x24;
	volatile uint32_t *GPIOx_AFR;
	int i;

	*RCC_AHB1ENR |= 1 << (bank - 'A');

	if (port >= 8) {
		GPIOx_AFR = GPIOx_AFRH;
		i = (port - 8) * 4;
	} else {
		GPIOx_AFR = GPIOx_AFRL;
		i = port * 4;
	}
	*GPIOx_AFR &= ~(GPIOx_AFRy_MASK << i);
	*GPIOx_AFR |= (uint32_t)altfunc << i;

	gpio_set(bank, port, otype, GPIOx_MODER_MODERy_ALTFUNC, ospeed, pupd);
}

static void gpio_set_fmc(char bank, uint8_t port)
{
	gpio_set_alt(bank, port, 0, GPIOx_OSPEEDR_OSPEEDRy_HIGH, 0, 0xC);
}

#define FMC_BASE	0xA0000000

#define FMC_SDSR_BUSY	(1 << 5)

static void fmc_wait_busy(void)
{
	volatile uint32_t *FMC_SDSR = (void *)(FMC_BASE + 0x158);

	while ((*FMC_SDSR & FMC_SDSR_BUSY)) {
	}
}

static void gpio_set_usart(char bank, uint8_t port)
{
	gpio_set_alt(bank, port, 0, GPIOx_OSPEEDR_OSPEEDRy_FAST, 1, 0x7);
}

void start_kernel(void)
{
	void (*kernel)(uint32_t reserved, uint32_t mach, uint32_t dt) = (void (*)(uint32_t, uint32_t, uint32_t))(0x08008000 | 1);

	kernel(0, ~0UL, 0x08004000);
}

#define SYSCFG_BASE	0x40013800

#define SYSCFG_MEMRMP_SWP_FMC	0x1

int main(void)
{
	volatile uint32_t *FLASH_KEYR = (void *)(FLASH_BASE + 0x04);
	volatile uint32_t *FLASH_CR = (void *)(FLASH_BASE + 0x10);
	volatile uint32_t *FMC_SDCR1 = (void *)(FMC_BASE + 0x140);
	volatile uint32_t *FMC_SDCR2 = (void *)(FMC_BASE + 0x144);
	volatile uint32_t *FMC_SDTR1 = (void *)(FMC_BASE + 0x148);
	volatile uint32_t *FMC_SDTR2 = (void *)(FMC_BASE + 0x14C);
	volatile uint32_t *FMC_SDCMR = (void *)(FMC_BASE + 0x150);
	volatile uint32_t *FMC_SDRTR = (void *)(FMC_BASE + 0x154);
	volatile uint32_t *SYSCFG_MEMRMP = (void *)(SYSCFG_BASE + 0x00);
	int i;

	if (*FLASH_CR & FLASH_CR_LOCK) {
		*FLASH_KEYR = 0x45670123;
		*FLASH_KEYR = 0xCDEF89AB;
	}
	*FLASH_CR &= ~(FLASH_CR_ERRIE | FLASH_CR_EOPIE | FLASH_CR_PSIZE_MASK);
	*FLASH_CR |= FLASH_CR_PSIZE_X32;
	*FLASH_CR |= FLASH_CR_LOCK;

	clock_setup();

	gpio_set_fmc('B', 5);
	gpio_set_fmc('B', 6);
	gpio_set_fmc('C', 0);
	gpio_set_fmc('D', 0);
	gpio_set_fmc('D', 1);
	gpio_set_fmc('D', 8);
	gpio_set_fmc('D', 9);
	gpio_set_fmc('D', 10);
	gpio_set_fmc('D', 14);
	gpio_set_fmc('D', 15);
	gpio_set_fmc('E', 0);
	gpio_set_fmc('E', 1);
	gpio_set_fmc('E', 7);
	gpio_set_fmc('E', 8);
	gpio_set_fmc('E', 9);
	gpio_set_fmc('E', 10);
	gpio_set_fmc('E', 11);
	gpio_set_fmc('E', 12);
	gpio_set_fmc('E', 13);
	gpio_set_fmc('E', 14);
	gpio_set_fmc('E', 15);
	gpio_set_fmc('F', 0);
	gpio_set_fmc('F', 1);
	gpio_set_fmc('F', 2);
	gpio_set_fmc('F', 3);
	gpio_set_fmc('F', 4);
	gpio_set_fmc('F', 5);
	gpio_set_fmc('F', 11);
	gpio_set_fmc('F', 12);
	gpio_set_fmc('F', 13);
	gpio_set_fmc('F', 14);
	gpio_set_fmc('F', 15);
	gpio_set_fmc('G', 0);
	gpio_set_fmc('G', 1);
	gpio_set_fmc('G', 4);
	gpio_set_fmc('G', 5);
	gpio_set_fmc('G', 8);
	gpio_set_fmc('G', 15);
	*FMC_SDCR1 = 0x00001800;
	*FMC_SDCR2 = 0x000019D4;
	*FMC_SDTR1 = 0x00106000;
	*FMC_SDTR2 = 0x00010361;

	fmc_wait_busy();
	*FMC_SDCMR = 0x00000009; // clock
	for (i = 0; i < 50000000; i++) { // 10 ms
		asm volatile ("nop");
	}
	fmc_wait_busy();
	*FMC_SDCMR = 0x0000000A; // PALL
	fmc_wait_busy();
	*FMC_SDCMR = 0x000000EB; // auto-refresh
	fmc_wait_busy();
	*FMC_SDCMR = 0x0004600C; // external memory mode
	*FMC_SDRTR = 1386 << 1; // refresh rate
	fmc_wait_busy();

	*SYSCFG_MEMRMP = SYSCFG_MEMRMP_SWP_FMC << 10;

	gpio_set_usart('A', 9);
	gpio_set_usart('A', 10);

	usart_setup(usart_base, 90000000);
	usart_putch(usart_base, '.');

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

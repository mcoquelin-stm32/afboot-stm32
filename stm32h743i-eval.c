#include <stdlib.h>
#include <stdint.h>

#include "stm32h7_regs.h"
#include "usart.h"
#include "gpio.h"
#include "mpu.h"
#include "qspi.h"
#include "start_kernel.h"

static void *usart_base = (void *)USART1_BASE;
static void *gpio_base = (void *)GPIOA_BASE;

void clock_setup(void)
{
	volatile uint32_t *RCC_CR = (void *)(RCC_BASE_REG);
	volatile uint32_t *RCC_CFGR = (void *)(RCC_BASE_REG + 0x10);
	volatile uint32_t *RCC_D1CFGR = (void *)(RCC_BASE_REG + 0x18);
	volatile uint32_t *RCC_D2CFGR = (void *)(RCC_BASE_REG + 0x1c);
	volatile uint32_t *RCC_D3CFGR = (void *)(RCC_BASE_REG + 0x20);
	volatile uint32_t *RCC_PLLCKSELR = (void *)(RCC_BASE_REG + 0x28);
	volatile uint32_t *RCC_PLLCFGR = (void *)(RCC_BASE_REG + 0x2c);
	volatile uint32_t *RCC_PLL1DIVR = (void *)(RCC_BASE_REG + 0x30);
	volatile uint32_t *RCC_PLL1FRACR = (void *)(RCC_BASE_REG + 0x34);
	volatile uint32_t *RCC_PLL2DIVR = (void *)(RCC_BASE_REG + 0x38);
	volatile uint32_t *RCC_PLL2FRACR = (void *)(RCC_BASE_REG + 0x3c);
	volatile uint32_t *RCC_PLL3DIVR = (void *)(RCC_BASE_REG + 0x40);
	volatile uint32_t *RCC_PLL3FRACR = (void *)(RCC_BASE_REG + 0x44);
	volatile uint32_t *RCC_D1CCIPR = (void *)(RCC_BASE_REG + 0x4c);
	volatile uint32_t *RCC_D2CCIP1R = (void *)(RCC_BASE_REG + 0x50);
	volatile uint32_t *RCC_D2CCIP2R = (void *)(RCC_BASE_REG + 0x54);
	volatile uint32_t *RCC_D1AHB1ENR = (void *)(RCC_BASE_REG + 0xd4);
	volatile uint32_t *RCC_D2AHB1ENR = (void *)(RCC_BASE_REG + 0xd8);
	volatile uint32_t *RCC_D2AHB2ENR = (void *)(RCC_BASE_REG + 0xdc);
	volatile uint32_t *RCC_D3AHB1ENR = (void *)(RCC_BASE_REG + 0xe0);
	volatile uint32_t *RCC_D1APB1ENR = (void *)(RCC_BASE_REG + 0xe4);
	volatile uint32_t *RCC_D2APB1LENR = (void *)(RCC_BASE_REG + 0xe8);
	volatile uint32_t *RCC_D2APB1HENR = (void *)(RCC_BASE_REG + 0xec);
	volatile uint32_t *RCC_D2APB2ENR = (void *)(RCC_BASE_REG + 0xf0);
	volatile uint32_t *RCC_D3APB1ENR = (void *)(RCC_BASE_REG + 0xf4);
	volatile uint32_t *RCC_AHB3RST = (void *)(RCC_BASE_REG + 0x7c);
	volatile uint32_t *FLASH_FACR = (void *)(FLASH_BASE);
	volatile uint32_t *PWR_D3CR = (void *)(PWR_BASE + 0x18);
	volatile uint32_t *PWR_CR3 = (void *)(PWR_BASE + 0xc);
	uint32_t divm, divn, divp, divq, divr;

	/*  enable HSI */
	*RCC_CR |= RCC_CR_HSION;

	/* Reset CFGR register */
	/* HSI by default as system clock*/
	*RCC_CFGR = 0;

	/*  reset registers ... */
	*RCC_D1CFGR = 0;
	*RCC_D2CFGR = 0;
	*RCC_D3CFGR = 0;
	*RCC_PLLCKSELR = 0;
	*RCC_PLLCFGR = 0;
	*RCC_PLL1DIVR = 0;
	*RCC_PLL1FRACR = 0;
	*RCC_PLL2DIVR = 0;
	*RCC_PLL2FRACR = 0;
	*RCC_PLL3DIVR = 0;
	*RCC_PLL3FRACR = 0;

	/* Activate all clock */
	*RCC_D3AHB1ENR = 0xFFFFFFFF;
	*RCC_D2APB2ENR = 0xFFFFFFFF;
	*RCC_D1AHB1ENR = 0xFFFFFFFF;
	*RCC_D2AHB1ENR = 0xFFFFFFFF;
	*RCC_D2AHB2ENR = 0xFFFFFFFF;
	*RCC_D1APB1ENR = 0xFFFFFFFF;
	*RCC_D2APB1LENR = 0xFFFFFFFF;
	*RCC_D2APB1HENR = 0xFFFFFFFF;
	*RCC_D3APB1ENR = 0xFFFFFFFF;
	*RCC_D1CCIPR = 0x00000000;
	*RCC_D2CCIP1R = 0x00000000;
	*RCC_D2CCIP2R = 0x00000000;

	/* Second level */
	*PWR_D3CR |= 0xc000;
	*PWR_CR3 &= ~(0x4);

	while (!(*PWR_D3CR & (1 <<13))) {
	}
	/* disable HSE to configure it  */
	*RCC_CR &= ~(RCC_CR_HSEON);
	while ((*RCC_CR & RCC_CR_HSERDY)) {
	}
	/* clear HSE bypass and set it ON */
	*RCC_CR &= (~RCC_CR_HSEBYP);
	*RCC_CR |=RCC_CR_HSEON;
	while (!(*RCC_CR & RCC_CR_HSERDY)) {
	}

	/* setup pll */
	/*  disable pll1 */
	*RCC_CR &= ~(RCC_CR_PLL1ON);
	while ((*RCC_CR & RCC_CR_PLL1RDY)) {
	}
	/* Configure PLL1 as clock source:
	 * OSC_HSE = 25 MHz
	 * VCO = 500MHz
	 * pll1_p = 250MHz / pll1_q = 250MHz*/
	divm = 4;
	divn = 80;
	divp = 2;
	divq = 2;
	divr = 2;

	/*  PLL SRC = HSE */
	*RCC_PLLCKSELR |= (divm << 4 ) | 0x2;
	*RCC_PLL1DIVR  |= ((divr - 1) << (24)) | ((divq - 1) << (16)) | ( (divp - 1) << 9) | (divn - 1);

	/*  Enable divP1, divQ1, divR1, pll1fracn */
	*RCC_PLLCFGR |= (2 << 2);
	*RCC_PLLCFGR |= (1 << 18) | (1 << 17) | (1 << 16);

	/*  enable the main PLL */
	*RCC_CR |= (1 << 24 );
	while (!(*RCC_CR & RCC_CR_PLL1RDY)) {
	}

	/*  set flash latency */
	*FLASH_FACR &=0xfffffff0;
	*FLASH_FACR |= 0xa;

	/* set HPRE (/2) DI clk --> 125MHz */
	*RCC_D1CFGR |= 8;

	/*  select PLL1 as clcok source */
	*RCC_CFGR |= 0x3;
	while ((((*RCC_CFGR)&0x3) != 0x3)) {
	}
	/*  test for sdram: use pll1_q as fmc_k clk */
	*RCC_D1CCIPR = 1 | (3 << 4);

	/* togle reset QSPI */
	*RCC_AHB3RST |= (1 << 14);
	*RCC_AHB3RST &= ~(1 << 14);

}

void ext_mem_setup(void)
{
	volatile uint32_t *RCC_D1AHB1RSTR = (void *)(RCC_BASE_REG + 0x7c);
	volatile uint32_t *FMC_BCR1 = (void *)(FMC_BASE);
	volatile uint32_t *FMC_SDCR1 = (void *)(FMC_BASE + 0x140);
	volatile uint32_t *FMC_SDCR2 = (void *)(FMC_BASE + 0x144);
	volatile uint32_t *FMC_SDTR1 = (void *)(FMC_BASE + 0x148);
	volatile uint32_t *FMC_SDTR2 = (void *)(FMC_BASE + 0x14c);
	volatile uint32_t *FMC_SDCMR = (void *)(FMC_BASE + 0x150);
	volatile uint32_t *FMC_SDRTR = (void *)(FMC_BASE + 0x154);
	volatile uint32_t *FMC_SDSR = (void *)(FMC_BASE + 0x158);
	uint32_t index, tmpreg = 0, timeout = 0xFFFF;

	/* Enable GPIOD, GPIOE, GPIOF, GPIOG, GPIOH and GPIOI interface
      clock */
	/* GPIOC  */
	gpio_set_fmc(gpio_base, 'C', 0);
	gpio_set_fmc(gpio_base, 'C', 2);
	gpio_set_fmc(gpio_base, 'C', 3);
	/* GPIOD  */
	gpio_set_fmc(gpio_base, 'D', 0);
	gpio_set_fmc(gpio_base, 'D', 1);
	gpio_set_fmc(gpio_base, 'D', 8);
	gpio_set_fmc(gpio_base, 'D', 9);
	gpio_set_fmc(gpio_base, 'D', 10);
	gpio_set_fmc(gpio_base, 'D', 14);
	gpio_set_fmc(gpio_base, 'D', 15);
	/* GPIOE  */
	gpio_set_fmc(gpio_base, 'E', 0);
	gpio_set_fmc(gpio_base, 'E', 1);
	gpio_set_fmc(gpio_base, 'E', 7);
	gpio_set_fmc(gpio_base, 'E', 8);
	gpio_set_fmc(gpio_base, 'E', 9);
	gpio_set_fmc(gpio_base, 'E', 10);
	gpio_set_fmc(gpio_base, 'E', 11);
	gpio_set_fmc(gpio_base, 'E', 12);
	gpio_set_fmc(gpio_base, 'E', 13);
	gpio_set_fmc(gpio_base, 'E', 14);
	gpio_set_fmc(gpio_base, 'E', 15);
	/* GPIOF  */
	gpio_set_fmc(gpio_base, 'F', 0);
	gpio_set_fmc(gpio_base, 'F', 1);
	gpio_set_fmc(gpio_base, 'F', 2);
	gpio_set_fmc(gpio_base, 'F', 3);
	gpio_set_fmc(gpio_base, 'F', 4);
	gpio_set_fmc(gpio_base, 'F', 5);
	gpio_set_fmc(gpio_base, 'F', 11);
	gpio_set_fmc(gpio_base, 'F', 12);
	gpio_set_fmc(gpio_base, 'F', 13);
	gpio_set_fmc(gpio_base, 'F', 14);
	gpio_set_fmc(gpio_base, 'F', 15);
	/* GPIOG  */
	gpio_set_fmc(gpio_base, 'G', 0);
	gpio_set_fmc(gpio_base, 'G', 1);
	gpio_set_fmc(gpio_base, 'G', 2);
	gpio_set_fmc(gpio_base, 'G', 3);
	gpio_set_fmc(gpio_base, 'G', 4);
	gpio_set_fmc(gpio_base, 'G', 5);
	gpio_set_fmc(gpio_base, 'G', 8);
	gpio_set_fmc(gpio_base, 'G', 15);
	/* GPIOH  */
	gpio_set_fmc(gpio_base, 'H', 2);
	gpio_set_fmc(gpio_base, 'H', 3);
	gpio_set_fmc(gpio_base, 'H', 5);
	gpio_set_fmc(gpio_base, 'H', 6);
	gpio_set_fmc(gpio_base, 'H', 7);
	gpio_set_fmc(gpio_base, 'H', 8);
	gpio_set_fmc(gpio_base, 'H', 9);
	gpio_set_fmc(gpio_base, 'H', 10);
	gpio_set_fmc(gpio_base, 'H', 11);
	gpio_set_fmc(gpio_base, 'H', 12);
	gpio_set_fmc(gpio_base, 'H', 13);
	gpio_set_fmc(gpio_base, 'H', 14);
	gpio_set_fmc(gpio_base, 'H', 15);
	/* GPIOI  */
	gpio_set_fmc(gpio_base, 'I', 0);
	gpio_set_fmc(gpio_base, 'I', 1);
	gpio_set_fmc(gpio_base, 'I', 2);
	gpio_set_fmc(gpio_base, 'I', 3);
	gpio_set_fmc(gpio_base, 'I', 4);
	gpio_set_fmc(gpio_base, 'I', 5);
	gpio_set_fmc(gpio_base, 'I', 6);
	gpio_set_fmc(gpio_base, 'I', 7);
	gpio_set_fmc(gpio_base, 'I', 9);
	gpio_set_fmc(gpio_base, 'I', 10);

	/*  Reset FMC */
	*RCC_D1AHB1RSTR &= 0xffffefff;
	*RCC_D1AHB1RSTR |= 0x1000;
	*RCC_D1AHB1RSTR &= 0xffffefff;

	/*FMC controller Enable*/
	*FMC_BCR1  |= 0x80000000;

	/* Bank2 config (SDCR2/SDTR2) is the most of time done by bank1 registers
	 * (SDCR1 /SDTR1) FMC_CLK/2 */
	*FMC_SDCR1 = 0x000019e5;
	*FMC_SDCR2 = 0x000019e5;

	*FMC_SDTR1 = 0x0fffffff;
	*FMC_SDTR2 = 0x0fffffff;

	/* SDRAM initialization sequence */
	/* Clock enable command */
	*FMC_SDCMR = 0x00000009;

	tmpreg = *FMC_SDSR & 0x00000020;
	while((tmpreg != 0) && (timeout-- > 0))
	{
		tmpreg = *FMC_SDSR & 0x00000020;
	}
	/* Delay */
	for (index = 0; index<1000; index++);

	/* PALL command */
	*FMC_SDCMR = 0x000000a;
	timeout = 0xFFFF;
	while((tmpreg != 0) && (timeout-- > 0))
	{
		tmpreg = *FMC_SDSR & 0x00000020;
	}

	*FMC_SDCMR = 0x000000eb;
	timeout = 0xFFFF;
	while((tmpreg != 0) && (timeout-- > 0))
	{
		tmpreg = *FMC_SDSR & 0x00000020;
	}

	*FMC_SDCMR = 0x0004400c;
	timeout = 0xFFFF;
	while((tmpreg != 0) && (timeout-- > 0))
	{
		tmpreg = *FMC_SDSR & 0x00000020;
	}
	/* Set refresh count */
	tmpreg = *FMC_SDRTR;
	*FMC_SDRTR = (tmpreg | (0x00000603<<1));

	tmpreg = *FMC_SDCR2;

	*FMC_SDCR1 = (tmpreg & 0xFFFFFDFF);
	*FMC_SDCR2 = (tmpreg & 0xFFFFFDFF);
}

#define SCB_BASE 0xe000ed00

static void clean_icache(void)
{
	volatile uint32_t *SCB_ICIALLU = (void *)(SCB_BASE + 0x250);

	asm volatile ("dsb");
	asm volatile ("isb");

	*SCB_ICIALLU = 0;

	asm volatile ("dsb");
	asm volatile ("isb");
}

static uint32_t clz(uint32_t data)
{
	uint32_t count = 0;
	uint32_t mask = 0x80000000;

	while((data & mask) == 0)
	{
		count += 1u;
		mask = mask >> 1u;
	}

	return (count);
}

static void clean_dcache(void)
{
	volatile uint32_t *SCB_CCSIDR = (void *)(SCB_BASE + 0x80);
	volatile uint32_t *SCB_CSSELR = (void *)(SCB_BASE + 0x84);
	volatile uint32_t *SCB_DCISW = (void *)(SCB_BASE + 0x260);
	uint32_t ccsidr, sshift, wshift, sets, ways, i, j;

	asm volatile ("dsb");
	asm volatile ("isb");

	*SCB_CSSELR = 0;

	ccsidr = *SCB_CCSIDR;
	sets = (ccsidr >> 13) & 0x7fff;
	sshift = (ccsidr & 7) + 4;
	ways = (ccsidr >> 3) & 0x3ff;
	wshift = clz(ways) & 0x1f;

	asm volatile ("dsb");
	for (i = 0; i < sets; i++)
		for (j = 0; j < ways; j++)
			*SCB_DCISW = ((j << wshift) | (i << sshift));

	asm volatile ("dsb");
	asm volatile ("isb");
}

int main(void)
{
	struct qspi_params qspi_h743_params = {
		.address_size = QUADSPI_CCR_ADSIZE_32BITS,
		.fifo_threshold = QUADSPI_CR_FTHRES(0),
		.sshift = QUADSPI_CR_SSHIFT,
		.fsize = QUADSPI_DCR_FSIZE_128MB,
		.prescaler = 0,
		.dummy_cycle = 8,
		.fsel = QUADSPI_CR_FSEL,
		.dfm = QUADSPI_CR_DFM,
	};

	mpu_config(0xd0000000);

	/* configure clocks */
	clock_setup();

	/* configure external memory controler */
	ext_mem_setup();

	gpio_set_qspi(gpio_base, 'B', 2, GPIOx_PUPDR_NOPULL, 0x9); //CLK
	/*  QSPI BANK1 */
	gpio_set_qspi(gpio_base, 'G', 6, GPIOx_PUPDR_PULLUP, 0xa); //CS
	gpio_set_qspi(gpio_base, 'F', 8, GPIOx_PUPDR_NOPULL, 0xa); //DO
	gpio_set_qspi(gpio_base, 'F', 9, GPIOx_PUPDR_NOPULL, 0xa); //D1
	gpio_set_qspi(gpio_base, 'F', 7, GPIOx_PUPDR_NOPULL, 0x9); //D2
	gpio_set_qspi(gpio_base, 'F', 6, GPIOx_PUPDR_NOPULL, 0x9); //D3
	/*  QSPI BANK2 */
	gpio_set_qspi(gpio_base, 'C', 11, GPIOx_PUPDR_PULLUP, 0x9); //CS
	gpio_set_qspi(gpio_base, 'H', 2, GPIOx_PUPDR_NOPULL, 0x9); //DO
	gpio_set_qspi(gpio_base, 'H', 3, GPIOx_PUPDR_NOPULL, 0x9); //D1
	gpio_set_qspi(gpio_base, 'G', 9, GPIOx_PUPDR_NOPULL, 0x9); //D2
	gpio_set_qspi(gpio_base, 'G', 14, GPIOx_PUPDR_NOPULL, 0x9); //D3

	quadspi_init(&qspi_h743_params, (void *)QUADSPI_BASE);

	gpio_set_usart(gpio_base, 'B', 14, 4);
	gpio_set_usart(gpio_base, 'B', 15, 4);

	usart_setup(usart_base, 125000000);
	usart_putch(usart_base, '.');

	clean_dcache();
	clean_icache();

	start_kernel();

	return 0;
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

static void noop(void)
{
	usart_putch(usart_base, 'E');
	while (1) {
	}
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

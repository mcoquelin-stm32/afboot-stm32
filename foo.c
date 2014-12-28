#include <stdlib.h>
#include <stdint.h>

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

#define RCC_APB1ENR_USART3EN	(1 << 18)

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

static void clock_setup(void)
{
	volatile uint32_t *RCC_CR = (void *)(RCC_BASE + 0x00);
	volatile uint32_t *RCC_PLLCFGR = (void *)(RCC_BASE + 0x04);
	volatile uint32_t *RCC_CFGR = (void *)(RCC_BASE + 0x08);
	volatile uint32_t *FLASH_ACR = (void *)(FLASH_BASE + 0x00);
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
#define GPIOG_BASE	0x40021800UL

#define GPIOx_MODER_MODERy_INPUT	0x0UL
#define GPIOx_MODER_MODERy_GPOUTPUT	0x1UL
#define GPIOx_MODER_MODERy_ALTFUNC	0x2UL
#define GPIOx_MODER_MODERy_ANALOG	0x3UL
#define GPIOx_MODER_MODERy_MASK		0x3UL

#define GPIOx_OSPEEDR_OSPEEDRy_FAST	0x2UL
#define GPIOx_OSPEEDR_OSPEEDRy_HIGH	0x3UL
#define GPIOx_OSPEEDR_OSPEEDRy_MASK	0x3UL

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

#define USART3_BASE	0x40004800

#define USART_SR_TXE	(1 << 7)

#define USART_CR1_RE	(1 << 2)
#define USART_CR1_TE	(1 << 3)
#define USART_CR1_UE	(1 << 13)

static void usart_setup(void)
{
	volatile uint32_t *RCC_APB1ENR = (void *)(RCC_BASE + 0x40);
	volatile uint32_t *USART3_BRR = (void *)(USART3_BASE + 0x08);
	volatile uint32_t *USART3_CR1 = (void *)(USART3_BASE + 0x0C);
	volatile uint32_t *USART3_CR2 = (void *)(USART3_BASE + 0x10);
	volatile uint32_t *USART3_CR3 = (void *)(USART3_BASE + 0x14);
	const uint32_t apb_clk_hz = 45000000;
	uint32_t int_div, frac_div, val;

	*RCC_APB1ENR |= RCC_APB1ENR_USART3EN;

	gpio_set_usart('C', 10);
	gpio_set_usart('C', 11);

	*USART3_CR1 = USART_CR1_TE | USART_CR1_RE;
	*USART3_CR2 = 0;
	*USART3_CR3 = 0;

	int_div = (25 * apb_clk_hz) / (4 * 115200);
	val = (int_div / 100) << 4;
	frac_div = int_div - 100 * (val >> 4);
	val |= ((frac_div * 16 + 50) / 100) & 0xf;
	*USART3_BRR = val;

	*USART3_CR1 |= USART_CR1_UE;
}

static void usart_putch(char ch)
{
	volatile uint32_t *USART3_SR  = (void *)(USART3_BASE + 0x00);
	volatile uint32_t *USART3_DR  = (void *)(USART3_BASE + 0x04);

	while (!(*USART3_SR & USART_SR_TXE)) {
	}
	*USART3_DR = ch;
}

void start_kernel(void);

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
	volatile uint32_t *RCC_AHB3ENR = (void *)(RCC_BASE + 0x38);
	volatile uint32_t *RCC_APB2ENR = (void *)(RCC_BASE + 0x44);
	volatile uint32_t *GPIOG_BSRR = (void *)(GPIOG_BASE + 0x18);
	volatile uint32_t *FMC_SDCR1 = (void *)(FMC_BASE + 0x140);
	volatile uint32_t *FMC_SDCR2 = (void *)(FMC_BASE + 0x144);
	volatile uint32_t *FMC_SDTR1 = (void *)(FMC_BASE + 0x148);
	volatile uint32_t *FMC_SDTR2 = (void *)(FMC_BASE + 0x14C);
	volatile uint32_t *FMC_SDCMR = (void *)(FMC_BASE + 0x150);
	volatile uint32_t *FMC_SDRTR = (void *)(FMC_BASE + 0x154);
	volatile uint32_t *SYSCFG_MEMRMP = (void *)(SYSCFG_BASE + 0x00);
	uint32_t *ptr;
	int i;

	if (*FLASH_CR & FLASH_CR_LOCK) {
		*FLASH_KEYR = 0x45670123;
		*FLASH_KEYR = 0xCDEF89AB;
	}
	*FLASH_CR &= ~(FLASH_CR_ERRIE | FLASH_CR_EOPIE | FLASH_CR_PSIZE_MASK);
	*FLASH_CR |= FLASH_CR_PSIZE_X32;
	*FLASH_CR |= FLASH_CR_LOCK;

	clock_setup();

	gpio_set('G', 13, 0, GPIOx_MODER_MODERy_GPOUTPUT,
		GPIOx_OSPEEDR_OSPEEDRy_FAST, 0);
	gpio_set('G', 14, 0, GPIOx_MODER_MODERy_GPOUTPUT,
		GPIOx_OSPEEDR_OSPEEDRy_FAST, 0);

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
	*RCC_AHB3ENR |= RCC_AHB3ENR_FMC;
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

	ptr = (void *)0xD0000000UL;
	i = 0x00800000UL / sizeof(*ptr);
	while (i-- > 0)
		*ptr++ = 0;

	*RCC_APB2ENR |= RCC_APB2ENR_SYSCFGEN;
	*SYSCFG_MEMRMP = SYSCFG_MEMRMP_SWP_FMC << 10;

	usart_setup();
	usart_putch('.');

	while (0) {
		*GPIOG_BSRR = (1 << 13) | (1 << (14 + 16));
		for (i = 0; i < 10000000; i++) {
			asm volatile ("nop");
		}
		*GPIOG_BSRR = (1 << 14) | (1 << (13 + 16));
		for (i = 0; i < 10000000; i++) {
			asm volatile ("nop");
		}
	}

	start_kernel();

	return 0;
}

static void noop(void)
{
	usart_putch('E');
	while (1) {
	}
}

extern unsigned int _end_text;
extern unsigned int _start_data;
extern unsigned int _end_data;
extern unsigned int _start_bss;
extern unsigned int _end_bss;

void reset(void);

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

#include <stdint.h>

#define USART_SR_TXE	(1 << 7)

#define USART_CR1_RE	(1 << 2)
#define USART_CR1_TE	(1 << 3)
#define USART_CR1_UE	(1 << 13)

void usart_setup(void *base, uint32_t clk_freq)
{
	volatile uint32_t *USART_BRR = base + 0x08;
	volatile uint32_t *USART_CR1 = base + 0x0C;
	volatile uint32_t *USART_CR2 = base + 0x10;
	volatile uint32_t *USART_CR3 = base + 0x14;
	uint32_t int_div, frac_div, val;

	*USART_CR1 = USART_CR1_TE | USART_CR1_RE;
	*USART_CR2 = 0;
	*USART_CR3 = 0;

	int_div = (25 * clk_freq) / (4 * 115200);
	val = (int_div / 100) << 4;
	frac_div = int_div - 100 * (val >> 4);
	val |= ((frac_div * 16 + 50) / 100) & 0xf;
	*USART_BRR = val;

	*USART_CR1 |= USART_CR1_UE;
}

void usart_putch(void *base, char ch)
{
	volatile uint32_t *USART_SR  = base + 0x00;
	volatile uint32_t *USART_DR  = base + 0x04;

	while (!(*USART_SR & USART_SR_TXE)) {
	}
	*USART_DR = ch;
}


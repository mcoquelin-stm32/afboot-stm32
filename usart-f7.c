#include <stdint.h>

#define USART_CR1_UE	(1 << 0)
#define USART_CR1_RE	(1 << 2)
#define USART_CR1_TE	(1 << 3)

#define USART_ISR_TXE	(1 << 7)

void usart_setup(void *base, uint32_t clk_freq)
{
	volatile uint32_t *USART_BRR = base + 0x0c;
	volatile uint32_t *USART_CR1 = base + 0x00;
	volatile uint32_t *USART_CR2 = base + 0x04;
	volatile uint32_t *USART_CR3 = base + 0x08;

	*USART_CR1 = USART_CR1_TE | USART_CR1_RE;
	*USART_CR2 = 0;
	*USART_CR3 = 0;

	*USART_BRR = clk_freq / 115200;

	*USART_CR1 |= USART_CR1_UE;
}

void usart_putch(void *base, char ch)
{
	volatile uint32_t *USART_ISR  = base + 0x1c;
	volatile uint32_t *USART_TDR  = base + 0x28;

	while (!(*USART_ISR & USART_ISR_TXE)) {
	}
	*USART_TDR = ch;
}


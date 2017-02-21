#include <stdint.h>

#include "gpio.h"

void gpio_set(void *base, char bank, uint8_t port,
	uint8_t otype, uint8_t mode, uint8_t ospeed, uint8_t pupd)
{
	volatile uint32_t *GPIOx_base    = (base + (bank - 'A') * 0x400);
	volatile uint32_t *GPIOx_MODER   = (void *)GPIOx_base + 0x00;
	volatile uint32_t *GPIOx_OTYPER  = (void *)GPIOx_base + 0x04;
	volatile uint32_t *GPIOx_OSPEEDR = (void *)GPIOx_base + 0x08;
	volatile uint32_t *GPIOx_PUPDR   = (void *)GPIOx_base + 0x0C;
	int i;

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

void gpio_set_alt(void *base, char bank, uint8_t port,
	uint8_t otype, uint8_t ospeed, uint8_t pupd, uint8_t altfunc)
{
	volatile uint32_t *GPIOx_base = (base + (bank - 'A') * 0x400);
	volatile uint32_t *GPIOx_AFRL = (void *)GPIOx_base + 0x20;
	volatile uint32_t *GPIOx_AFRH = (void *)GPIOx_base + 0x24;
	volatile uint32_t *GPIOx_AFR;
	int i;

	if (port >= 8) {
		GPIOx_AFR = GPIOx_AFRH;
		i = (port - 8) * 4;
	} else {
		GPIOx_AFR = GPIOx_AFRL;
		i = port * 4;
	}
	*GPIOx_AFR &= ~(GPIOx_AFRy_MASK << i);
	*GPIOx_AFR |= (uint32_t)altfunc << i;

	gpio_set(base, bank, port, otype, GPIOx_MODER_MODERy_ALTFUNC, ospeed, pupd);
}

void gpio_set_fmc(void *base, char bank, uint8_t port)
{
	gpio_set_alt(base, bank, port, 0, GPIOx_OSPEEDR_OSPEEDRy_HIGH, 0, 0xC);
}

void gpio_set_qspi(void *base, char bank, uint8_t port, uint8_t pupd, uint8_t altfunc)
{
	gpio_set_alt(base, bank, port, 0, GPIOx_OSPEEDR_OSPEEDRy_HIGH, pupd, altfunc);
}

void gpio_set_usart(void *base, char bank, uint8_t port, uint8_t altfunc)
{
	gpio_set_alt(base, bank, port, 0, GPIOx_OSPEEDR_OSPEEDRy_FAST, 1, altfunc);
}



#ifndef _GPIO_H
#define _GPIO_H

#define GPIOA_BASE	0x40020000UL

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

void gpio_set(char bank, uint8_t port, uint8_t otype, uint8_t mode,
		uint8_t ospeed, uint8_t pupd);
void gpio_set_alt(char bank, uint8_t port, uint8_t otype, uint8_t ospeed,
		uint8_t pupd, uint8_t altfunc);
void gpio_set_fmc(char bank, uint8_t port);
void gpio_set_usart(char bank, uint8_t port);

#endif /* _GPIO_H */

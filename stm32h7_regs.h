#ifndef _STM32H7_REGS_H
#define _STM32H7_REGS_H

/*  RCC */
#define RCC_BASE_REG		0x58024400

#define RCC_CR_HSION			1
#define RCC_CR_HSEON			1<<16
#define RCC_CR_HSERDY			1<<17
#define RCC_CR_HSEBYP			1<<18
#define RCC_CR_PLL1ON			1<<24
#define RCC_CR_PLL1RDY			1<<25
#define RCC_D1AHB1ENR_FMCEN		1<<12

/*  PWR */
#define PWR_BASE			0x58024800
/*  FLASH */
#define FLASH_BASE			0x52002000
#define FLASH_FACR_REG		FLASH_BASE
/*  FMC */
#define FMC_BASE			0x52004000
#define	FMC_BCR1_REG		FMC_BASE

/*  Other base */
#define USART1_BASE			0x40011000
#define GPIOA_BASE			0x58020000UL
#define SDRAM_BASE			0xd0000000UL
#define QUADSPI_BASE		0x52005000

#endif /* _STM32H7_REGS_H */

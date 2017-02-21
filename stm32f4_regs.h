#ifndef _STM32F4_REGS_H
#define _STM32F4_REGS_H

#define RCC_BASE	0x40023800

#define RCC_CR_HSEON	(1 << 16)
#define RCC_CR_HSERDY	(1 << 17)
#define RCC_CR_PLLON	(1 << 24)
#define RCC_CR_PLLRDY	(1 << 25)
#define RCC_CR_SAION	(1 << 28)
#define RCC_CR_SAIRDY	(1 << 29)

#define RCC_PLLCFGR_PLLSRC_HSE	(1 << 22)

#define RCC_CFGR_SW_PLL		(0x2 << 0)
#define RCC_CFGR_SW_MASK	(0x3 << 0)
#define RCC_CFGR_SWS_PLL	(0x2 << 2)
#define RCC_CFGR_SWS_MASK	(0x3 << 2)
#define RCC_CFGR_HPRE_MASK	(0xf << 4)
#define RCC_CFGR_PPRE1_MASK	(0x7 << 10)
#define RCC_CFGR_PPRE2_MASK	(0x7 << 13)

#define RCC_DCKCFGR_48SRC_SAI	(1 << 27)
#define RCC_DCKCFGR_SDIO48	(1 << 28)

#define RCC_AHB1ENR_GPIOGEN	(1 << 6)

#define RCC_AHB1LPENR_OTGHSULPILPEN	(1 << 30)

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

#define USART3_BASE	0x40004800
#define USART1_BASE	0x40011000
#define GPIOA_BASE	0x40020000

#define FMC_BASE	0xA0000000
#define QUADSPI_BASE	0xA0001000

#define FMC_SDSR_BUSY	(1 << 5)

#define SYSCFG_BASE	0x40013800
#define SYSCFG_MEMRMP_SWP_FMC	0x1


#endif /* _STM32F4_REGS_H */

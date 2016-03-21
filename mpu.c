#include <stdint.h>

#define MPU_BASE			0xe000ed90

#define MPU_CTLR_ENABLE		(1 << 0)
#define MPU_CTLR_PRIVDEFENA	(1 << 2)

#define MPU_RBAR_ADDR_MASK	0xffffffe0

#define MPU_RASR_AP_SHIFT	24
#define MPU_RASR_AP_RWRW	(0x3 << MPU_RASR_AP_SHIFT)

#define MPU_RASR_TEX_SHIFT  19
#define MPU_RASR_S_SHIFT	18
#define MPU_RASR_C_SHIFT	17
#define MPU_RASR_B_SHIFT	16
#define MPU_RASR_SIZE_SHIFT 1

#define MPU_RASR_REG_ENABLE 0x1

#define MPU_RASR_SIZE_256M (28 << MPU_RASR_SIZE_SHIFT)
#define MPU_RASR_SIZE_32M (24 << MPU_RASR_SIZE_SHIFT)
#define MPU_RASR_SIZE_1M (19 << MPU_RASR_SIZE_SHIFT)


#define MPU_RASR_PERM_ATTRS(TEX, C, B, S) \
	((TEX << MPU_RASR_TEX_SHIFT) |\
	 (C << MPU_RASR_C_SHIFT) | \
	 (B << MPU_RASR_B_SHIFT) | \
	 (S << MPU_RASR_S_SHIFT))

#define MPU_RASR_INT_FLASH_ATTR		MPU_RASR_PERM_ATTRS(0, 1, 0, 0)
#define MPU_RASR_INT_RAM_ATTR		MPU_RASR_PERM_ATTRS(0, 1, 0, 1)
#define MPU_RASR_EXT_RAM_ATTR		MPU_RASR_PERM_ATTRS(0, 1, 1, 0)
#define MPU_RASR_PERIPH_ATTR		MPU_RASR_PERM_ATTRS(0, 0, 1, 1)
#define MPU_RASR_PRIV_PERIPH_ATTR	MPU_RASR_PERM_ATTRS(0, 0, 0, 0)

void mpu_config(uint32_t sdram_base)
{
	volatile uint32_t *MPU_CTRL = (void *)(MPU_BASE + 0x04);
	volatile uint32_t *MPU_RNR  = (void *)(MPU_BASE + 0x08);
	volatile uint32_t *MPU_RBAR = (void *)(MPU_BASE + 0x0c);
	volatile uint32_t *MPU_RASR = (void *)(MPU_BASE + 0x10);

	/* Configure external SDRAM */
	*MPU_RNR	= 0x0;
	*MPU_RBAR	= sdram_base;
	*MPU_RASR	= MPU_RASR_AP_RWRW |
					MPU_RASR_EXT_RAM_ATTR |
					MPU_RASR_SIZE_256M |
					MPU_RASR_REG_ENABLE;

	*MPU_RNR	= 0x2;
	*MPU_RBAR	= 0x90000000;
	*MPU_RASR	= MPU_RASR_AP_RWRW |
					MPU_RASR_EXT_RAM_ATTR|
					MPU_RASR_SIZE_32M |
					MPU_RASR_REG_ENABLE;

	*MPU_CTRL	= MPU_CTLR_ENABLE | MPU_CTLR_PRIVDEFENA;

	asm volatile ("dsb");
	asm volatile ("isb");
}

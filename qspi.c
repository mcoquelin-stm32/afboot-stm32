#include <stdint.h>

#include "qspi.h"

void quadspi_busy_wait(void)
{
	volatile uint32_t *QUADSPI_SR		= (void *)QUADSPI_BASE + 0x08;

	while (*QUADSPI_SR & QUADSPI_SR_BUSY);
}

void quadspi_wait_flag(uint32_t flag)
{
	volatile uint32_t *QUADSPI_SR		= (void *)QUADSPI_BASE + 0x08;
	volatile uint32_t *QUADSPI_FCR		= (void *)QUADSPI_BASE + 0x0c;

	while (!(*QUADSPI_SR & flag));
	*QUADSPI_FCR = flag;
}

void quadspi_write_enable(void)
{
	volatile uint32_t *QUADSPI_CR		= (void *)QUADSPI_BASE + 0x00;
	volatile uint32_t *QUADSPI_DLR		= (void *)QUADSPI_BASE + 0x10;
	volatile uint32_t *QUADSPI_CCR		= (void *)QUADSPI_BASE + 0x14;
	volatile uint32_t *QUADSPI_PSMKR	= (void *)QUADSPI_BASE + 0x24;
	volatile uint32_t *QUADSPI_PSMAR	= (void *)QUADSPI_BASE + 0x28;
	volatile uint32_t *QUADSPI_PIR		= (void *)QUADSPI_BASE + 0x2c;

	quadspi_busy_wait();

	*QUADSPI_CCR = QUADSPI_CCR_FMODE_IND_WR | QUADSPI_CCR_IDMOD_1_LINE |
		WRITE_ENABLE_CMD;

	quadspi_wait_flag(QUADSPI_SR_TCF);

	quadspi_busy_wait();

	*QUADSPI_PSMAR = N25Q512A_SR_WREN;
	*QUADSPI_PSMKR = N25Q512A_SR_WREN;
	*QUADSPI_PIR = 0x10;

	*QUADSPI_CR |= QUADSPI_CR_AMPS;
	*QUADSPI_DLR = 0;
	*QUADSPI_CCR = QUADSPI_CCR_FMODE_AUTO_POLL | QUADSPI_CCR_DMODE_1_LINE |
		QUADSPI_CCR_IDMOD_1_LINE | READ_STATUS_REG_CMD;

	quadspi_wait_flag(QUADSPI_SR_SMF);
}

void quadspi_init(struct qspi_params *params)
{
	volatile uint32_t *QUADSPI_CR		= (void *)QUADSPI_BASE + 0x00;
	volatile uint32_t *QUADSPI_DCR		= (void *)QUADSPI_BASE + 0x04;
	volatile uint32_t *QUADSPI_SR		= (void *)QUADSPI_BASE + 0x08;
	volatile uint32_t *QUADSPI_DLR		= (void *)QUADSPI_BASE + 0x10;
	volatile uint32_t *QUADSPI_CCR		= (void *)QUADSPI_BASE + 0x14;
	volatile uint32_t *QUADSPI_AR		= (void *)QUADSPI_BASE + 0x18;
	volatile uint32_t *QUADSPI_DR		= (void *)QUADSPI_BASE + 0x20;
	volatile uint32_t *QUADSPI_PSMKR	= (void *)QUADSPI_BASE + 0x24;
	volatile uint32_t *QUADSPI_PSMAR	= (void *)QUADSPI_BASE + 0x28;
	volatile uint32_t *QUADSPI_PIR		= (void *)QUADSPI_BASE + 0x2c;
	volatile uint32_t *QUADSPI_LPTR		= (void *)QUADSPI_BASE + 0x30;
	uint32_t reg;

	*QUADSPI_CR = params->fifo_threshold;

	quadspi_busy_wait();

    *QUADSPI_CR |= QUADSPI_CR_PRESCALER(1) | QUADSPI_CR_SSHIFT;
    *QUADSPI_DCR = QUADSPI_DCR_FSIZE_64MB | QUADSPI_DCR_CSHT(1);

    *QUADSPI_CR |= QUADSPI_CR_EN;

	/* Reset memory */
	quadspi_busy_wait();

	*QUADSPI_CCR = QUADSPI_CCR_FMODE_IND_WR | QUADSPI_CCR_IDMOD_1_LINE |
		RESET_ENABLE_CMD;

	quadspi_wait_flag(QUADSPI_SR_TCF);

	*QUADSPI_CCR = QUADSPI_CCR_FMODE_IND_WR | QUADSPI_CCR_IDMOD_1_LINE |
		RESET_MEMORY_CMD;

	quadspi_wait_flag(QUADSPI_SR_TCF);

	quadspi_busy_wait();

	*QUADSPI_PSMAR = 0;
	*QUADSPI_PSMKR = N25Q512A_SR_WIP;
	*QUADSPI_PIR = 0x10;

	*QUADSPI_CR |= QUADSPI_CR_AMPS;
	*QUADSPI_DLR = 0;
	*QUADSPI_CCR = QUADSPI_CCR_FMODE_AUTO_POLL | QUADSPI_CCR_DMODE_1_LINE |
		QUADSPI_CCR_IDMOD_1_LINE | READ_STATUS_REG_CMD;

	quadspi_wait_flag(QUADSPI_SR_SMF);

	/* Enter 4-bytes address mode */
	quadspi_write_enable();

	quadspi_busy_wait();

	*QUADSPI_CCR = QUADSPI_CCR_FMODE_IND_WR | QUADSPI_CCR_IDMOD_1_LINE |
		ENTER_4_BYTE_ADDR_MODE_CMD;

	quadspi_wait_flag(QUADSPI_SR_TCF);

	quadspi_busy_wait();

	*QUADSPI_PSMAR = 0;
	*QUADSPI_PSMKR = N25Q512A_SR_WIP;
	*QUADSPI_PIR = 0x10;

	*QUADSPI_CR |= QUADSPI_CR_AMPS;
	*QUADSPI_DLR = 0;
	*QUADSPI_CCR = QUADSPI_CCR_FMODE_AUTO_POLL | QUADSPI_CCR_DMODE_1_LINE |
		QUADSPI_CCR_IDMOD_1_LINE | READ_STATUS_REG_CMD;

	quadspi_wait_flag(QUADSPI_SR_SMF);

	/* Configure dummy cycles on memory side */

	quadspi_busy_wait();

	*QUADSPI_DLR = 0;
	*QUADSPI_CCR = QUADSPI_CCR_FMODE_IND_WR | QUADSPI_CCR_DMODE_1_LINE |
		QUADSPI_CCR_IDMOD_1_LINE | READ_VOL_CFG_REG_CMD;

	*QUADSPI_CCR |= QUADSPI_CCR_FMODE_IND_RD;

	*QUADSPI_AR = *QUADSPI_AR; //Needed?

	while (!(*QUADSPI_SR & (QUADSPI_SR_FTF | QUADSPI_SR_TCF)));

	reg = *QUADSPI_DR;

	quadspi_wait_flag(QUADSPI_SR_TCF);

	reg = (reg & ~0xf0) | 0xa0; // N25Q512A dummy cycle value

	quadspi_write_enable();

	quadspi_busy_wait();

	*QUADSPI_DLR = 0;
	*QUADSPI_CCR = QUADSPI_CCR_FMODE_IND_WR | QUADSPI_CCR_DMODE_1_LINE |
		QUADSPI_CCR_IDMOD_1_LINE | WRITE_VOL_CFG_REG_CMD;

	while (!(*QUADSPI_SR & QUADSPI_SR_FTF));

	*QUADSPI_DR = reg;

	quadspi_wait_flag(QUADSPI_SR_TCF);

	quadspi_busy_wait();

	*QUADSPI_CR |= QUADSPI_CR_TCEN;
	*QUADSPI_LPTR = 1;

	*QUADSPI_CCR = QUADSPI_CCR_FMODE_MEMMAP | QUADSPI_CCR_DMODE_4_LINES |
		QUADSPI_CCR_DCYC_N25Q12A | params->address_size |
		QUADSPI_CCR_ADMOD_1_LINE | QUADSPI_CCR_IDMOD_1_LINE |
		QUAD_OUTPUT_FAST_READ_CMD;

	quadspi_busy_wait();
}


/*
 * MCP20137 implementation
 * Datasheet: http://ww1.microchip.com/downloads/en/devicedoc/20001952c.pdf
 *
 * Set up the MCP23017 IC as an i2c-base expander to support 8 button inputs
 */

#include <stdio.h>

#include "mcp23017.h"
#include "i2cmaster.h"

uint8_t mcp23017_init(char** msg) {
	uint8_t iocon = 0;
#ifdef USE_MCP23017_BANK
	iocon |= (1<<IOCON_BANK);
#endif
#ifdef USE_EXTERNAL_INTERRUPT
	iocon |= (1<<IOCON_INTPOL);
#endif

	mcp23017_write_reg(IOCON, iocon);
	mcp23017_write_reg(IODIRA, 0xFF);
	mcp23017_write_reg(IPOLA, 0xFF);
	mcp23017_write_reg(GPPUA, 0xFF);

#ifdef USE_EXTERNAL_INTERRUPT
	mcp23017_write_reg(GPINTENA, 0xFF);
	//mcp23017_write_reg(DEFVALA, 0xFF);
	mcp23017_write_reg(INTCONA, 0x00);
#endif
	mcp23017_read_reg(GPIOA);
	mcp23017_read_reg(INTCAPA);
	return 0;
}

void mcp23017_write_reg(uint8_t reg, uint8_t data) {
	i2c_start(MCP23017_ADDR + I2C_WRITE);
	i2c_write(reg);
	i2c_write(data);
	i2c_stop();
}

uint8_t mcp23017_read_reg(uint8_t reg) {
	i2c_start_wait(MCP23017_ADDR + I2C_WRITE);
	i2c_write(reg);
	i2c_rep_start(MCP23017_ADDR + I2C_READ);
	uint8_t data = i2c_readNak();
	i2c_stop();

	return data;
}
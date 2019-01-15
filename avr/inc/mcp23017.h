/*
 * MCP23017 Interface
 */

#ifndef MCP23017_INTERFACE_H_
#define MCP23017_INTERFACE_H_

#define MCP23017_ADDR 0b01000000

#define IOCON_BANK 7
#define IOCON_MIRROR 6
#define IOCON_SEQOP 5
#define IOCON_DISSLW 4
#define IOCON_HAEN 3
#define IOCON_ODR 2
#define IOCON_INTPOL 1

#define P7 0x80
#define P6 0x40
#define P5 0x20
#define P4 0x10
#define P3 0x08
#define P2 0x04
#define P1 0x02
#define P0 0x01

#ifdef USE_MCP23017_BANK
#define IODIRA   0x00
#define IPOLA    0x01
#define GPINTENA 0x02
#define DEFVALA  0x03
#define INTCONA  0x04
#define IOCON    0x05
#define GPPUA    0x06
#define INTFA    0x07
#define INTCAPA  0x08
#define GPIOA    0x09
#define OLATA    0x0A
#define IODIRB   0x10
#define IPOLB    0x11
#define GPINTENB 0x12
#define DEFVALB  0x13
#define INTCONB  0x14
#define IOCONB   0x15
#define GPPUB    0x16
#define INTFB    0x17
#define INTCAPB  0x18
#define GPIOB    0x19
#define OLATB    0x1A

#else
#define IODIRA   0x00
#define IODIRB   0x01
#define IPOLA    0x02
#define IPOLB    0x03
#define GPINTENA 0x04
#define GPINTENB 0x05
#define DEFVALA  0x06
#define DEFVALB  0x07
#define INTCONA  0x08
#define INTCONB  0x09
#define IOCON    0x0A
#define GPPUA    0x0C
#define GPPUB    0x0D
#define INTFA    0x0E
#define INTFB    0x0F
#define INTCAPA  0x10
#define INTCAPB  0x11
#define GPIOA    0x12
#define GPIOB    0x13
#define OLATA    0x14
#define OLATB    0x15

#endif

typedef struct __attribute__ ((packed))
_iocon_reg_t {
	/*!
	 * BANK: Controls how the registers are addressed
	 * 1 = The registers associated with each port are separated into different banks.
	 * 0 = The registers are in the same bank (addresses are sequential).
	 */
	unsigned int bank:1;

	/*!
	 * MIRROR: INT Pins Mirror bit
	 * 1 = The INT pins are internally connected
	 * 0 = The INT pins are not connected. INTA is associated with PORTA and INTB is associated with PORTB
	 */
	unsigned int mirror:1;

	/*!
	 * SEQOP: Sequential Operation mode bit
	 * 1 = Sequential operation disabled, address pointer does not increment.
	 * 0 = Sequential operation enabled, address pointer increments
	 */
	unsigned int seqop:1;

	/*!
	 * DISSLW: Slew Rate control bit for SDA output
	 * 1 = Slew rate disabled
	 * 0 = Slew rate enabled
	 */
	unsigned int disslw:1;

	/*!
	 * HAEN: Hardware Address Enable bit (MCP23S17 only)
	 * 1 = Enables the MCP23S17 address pins.
	 * 0 = Disables the MCP23S17 address pins.
	 * (Address pins are always enabled on the MCP23017.)
	 */
	unsigned int haen:1;

	/*!
	 * ODR: Configures the INT pin as an open-drain output
	 * 1 = Open-drain output (overrides the INTPOL bit.)
	 * 0 = Active driver output (INTPOL bit sets the polarity.)
	 */
	unsigned int odr:1;

	/*!
	 * INTPOL: This bit sets the polarity of the INT output pin
	 * 1 = Active-high
	 * 0 = Active-low
	 */
	unsigned int intpol:1;

	/*!
	 * Unimplemented: Read as '0'
	 */
	unsigned int res:1;
} iocon_reg_t;

uint8_t mcp23017_init(char** msg);
void mcp23017_write_reg(uint8_t reg, uint8_t data);
uint8_t mcp23017_read_reg(uint8_t reg);
#endif
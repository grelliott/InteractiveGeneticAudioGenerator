/*
             LUFA Library
     Copyright (C) Dean Camera, 2017.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
*/

/*
  Copyright 2017  Dean Camera (dean [at] fourwalledcubicle [dot] com)

  Permission to use, copy, modify, distribute, and sell this
  software and its documentation for any purpose is hereby granted
  without fee, provided that the above copyright notice appear in
  all copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaims all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
*/

/** \file
 *
 *  Main source file for the VirtualSerial demo. This file contains the main tasks of
 *  the demo and is responsible for the initial application hardware configuration.
 */

#include "VirtualSerial.h"
//#include "midi.h"

/** LUFA CDC Class driver interface configuration and state information. This structure is
 *  passed to all CDC Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another.
 */
USB_ClassInfo_CDC_Device_t VirtualSerial_CDC_Interface =
	{
		.Config =
			{
				.ControlInterfaceNumber   = INTERFACE_ID_CDC_CCI,
				.DataINEndpoint           =
					{
						.Address          = CDC_TX_EPADDR,
						.Size             = CDC_TXRX_EPSIZE,
						.Banks            = 1,
					},
				.DataOUTEndpoint =
					{
						.Address          = CDC_RX_EPADDR,
						.Size             = CDC_TXRX_EPSIZE,
						.Banks            = 1,
					},
				.NotificationEndpoint =
					{
						.Address          = CDC_NOTIFICATION_EPADDR,
						.Size             = CDC_NOTIFICATION_EPSIZE,
						.Banks            = 1,
					},
			},
	};

/** Standard file stream for the CDC interface when set up, so that the virtual CDC COM port can be
 *  used like any regular character stream in the C APIs.
 */
static FILE USBSerialStream;
static char statusBuffer[1024] = "";
static bool hostReady = false;
volatile uint8_t buttons = 0;  // data buffer for sending to SPI
uint8_t ret;
#define DEBOUNCE_MS 50

void logStatus(char* msg)
{
	if (hostReady) {
		fputs(msg, &USBSerialStream);
	} else {
		strcat(statusBuffer, msg);
	}
}

/*
 * It looks like that I may need to do the circuit so the uc is run at 3.3v instead of 5v
 * At least, this seems to be the recommended way to interface with the rPi
 * This would mean running at 8MHz instead of 16MHz
 * If I want to have USB, I'd need to consider a separate 5v circuit for it
 * The MCP can run at 3.3v
 * The LEDs and switches can use 3.3v
 * see http://robotics.hobbizine.com/raspiduino.html for info on rPi <-> uc using SPI
 */

/** Main program entry point. This routine contains the overall program flow, including initial
 *  setup of all components and the main program loop.
 */
int main(void)
{
	char* errMsg = "";
	uint8_t bufLen = 80;
	char buf[bufLen];


	logStatus("MIDI Interface started\n\r");

	SetupHardware();
	logStatus("Serial comms initialized\n\r");

	/* Create a regular character stream for the interface so that it can be used with the stdio.h functions */
	CDC_Device_CreateStream(&VirtualSerial_CDC_Interface, &USBSerialStream);
	LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);
	GlobalInterruptEnable();

#ifdef USE_EXTERNAL_INTERRUPT
	// Set up INT2 (PD2) up as external interrupt
	logStatus("Initializing external interrupt\n\r");
	DDRD |= (1 << PIND2);
	PORTD |= (1 << PIND2);
	EIMSK |= (1 << INT2);
	EIFR |= (1 << INTF2);
	EICRA |= (1 << ISC21) | (1 << ISC20);
	logStatus("External interrupt initialized\n\r");
#endif

	logStatus("Initializing I2C\n\r");
	i2c_init();
	logStatus("I2C initialized\n\r");

	logStatus("Initializing MCP23017\n\r");
	uint8_t mcpResult = mcp23017_init(&errMsg);
	if (mcpResult == 0) {
		logStatus("MCP23017 initialized successfully\n\r");
	} else {
		snprintf(buf, bufLen, "Failed to initialize MCP23107: %s\n\r", errMsg);
		logStatus(buf);
	}

	// Initialize SPI as slave device
	// RPi only operates as SPI master, so we must be a slave
	//
	// Pins
	//  PB3 MISO Slave data out
	//  PB2 MOSI Slave data input
	//  PB1 SCK Slave clock input
	//  PB0 SS Slave port select
	//  -  When the SPI is enabled as a slave, this pin is configured as an input regardless of
	//    the setting of DDB0. As a slave, the SPI is activated when this pin is driven low
	//
	// Registers
	//  PRR0 (Power reduction register)
	//  - Bit 2 PRSPI
	//  - Write 0 to enable SPI module
	//  - Write a 1 shuts down SPI. When waking up, reinitialize SPI
	//
	//  SPCR - SPI Control Register
	//  SPSR - SPI Status Register
	//  SPDR - SPI Data Register
	//
	// Interrupt handlers
	//  INT(SPI) SPI Serial Transfer complete

	// Enable SPI by writing 0 to PRSPI bit (2) in PRR0 register
	// The SPI Master initiates the communication cycle when pulling low the Slave Select SS pin of the desired Slave
	logStatus("Initializing SPI slave\n\r");
	// set MISO out
	// set MOSI in
	// set SCK in
	// set SS in
	DDR_SPI |= (1<<DD_MISO);
	DDR_SPI &= ~(1<<DD_MOSI);
	DDR_SPI &= ~(1<<DD_SCK);
	DDR_SPI &= ~(1<<DD_SS);
	// Enable SPI, enable interrupt
	SPCR = (1<<SPE) | (1<<SPIE);
	// uint8_t spsr = SPSR;
	// uint8_t spdr = SPDR;
	// (void)spsr;
	// (void)spdr;
	logStatus("SPI slave initialized\n\r");


	while (1) {
		/* Must throw away unused bytes from the host, or it will lock up while waiting for the device */
		CDC_Device_ReceiveByte(&VirtualSerial_CDC_Interface);
		CDC_Device_USBTask(&VirtualSerial_CDC_Interface);
		USB_USBTask();

#ifdef USE_EXTERNAL_INTERRUPT
	//TODO Use I2C interrupts instead
	if (ret != 0) {
		for (int i = 0; i < sizeof(ret)*8; i++) {
			if (ret & 1 << i) {
				snprintf(buf, bufLen, "Button %d pressed\n\r", i);
				buttons |= (1 << i);

				//buttons[i] = 0x01;
				// add button number to SPI send buffer
				//SPDR = i;
				// wait for transmission complete
				//while (!(SPSR & (1<<SPIF)));
				// create a MIDI note
				//midi_t* note_on = Midi(i, 1);
				// transmit note_on
				// wait a moment
				// make note_off
				// transmit note_off

				break;
			}
		}
		logStatus(buf);
		ret = 0;
	}
#else
		ret = mcp23017_read_reg(GPIOA);
		if (ret != 0) {
			snprintf(buf, bufLen, "0x%02x\n\r", ret);
			logStatus(buf);
		}
#endif
	}
}

#ifdef USE_EXTERNAL_INTERRUPT
ISR (INT2_vect) {
	// we received an interrupt from the MCP23017
	ret = mcp23017_read_reg(INTCAPA);
	// re-enable interrupt
	EIFR |= (1 << INTF2);
}
#endif

ISR (SPI_STC_vect) {
	uint8_t command;
	command = SPDR;
	if (command == 0x80) {
		SPDR = buttons;
		buttons = 0;
	} else {
		SPDR = 0;
	}
}

/** Configures the board hardware and chip peripherals for the USB functionality. */
void SetupHardware(void) {
	/* Disable watchdog if enabled by bootloader/fuses */
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	/* Disable clock division */
	clock_prescale_set(clock_div_1);

	/* Hardware Initialization */
	LEDs_Init();
	USB_Init();
}

/** Event handler for the library USB Connection event. */
void EVENT_USB_Device_Connect(void) {
	LEDs_SetAllLEDs(LEDMASK_USB_ENUMERATING);
}

/** Event handler for the library USB Disconnection event. */
void EVENT_USB_Device_Disconnect(void) {
	LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);
}

/** Event handler for the library USB Configuration Changed event. */
void EVENT_USB_Device_ConfigurationChanged(void) {
	bool ConfigSuccess = true;
	ConfigSuccess &= CDC_Device_ConfigureEndpoints(&VirtualSerial_CDC_Interface);
	LEDs_SetAllLEDs(ConfigSuccess ? LEDMASK_USB_READY : LEDMASK_USB_ERROR);
}

/** Event handler for the library USB Control Request reception event. */
void EVENT_USB_Device_ControlRequest(void) {
	CDC_Device_ProcessControlRequest(&VirtualSerial_CDC_Interface);
}

/** CDC class driver callback function the processing of changes to the virtual
 *  control lines sent from the host..
 *
 *  \param[in] CDCInterfaceInfo  Pointer to the CDC class interface configuration structure being referenced
 */
void EVENT_CDC_Device_ControLineStateChanged(USB_ClassInfo_CDC_Device_t *const CDCInterfaceInfo) {
	/* You can get changes to the virtual CDC lines in this callback; a common
	   use-case is to use the Data Terminal Ready (DTR) flag to enable and
	   disable CDC communications in your application when set to avoid the
	   application blocking while waiting for a host to become ready and read
	   in the pending data from the USB endpoints.
	*/
	hostReady = (CDCInterfaceInfo->State.ControlLineStates.HostToDevice & CDC_CONTROL_LINE_OUT_DTR) != 0;
	if (hostReady && strlen(statusBuffer) > 0) {
		fputs(statusBuffer, &USBSerialStream);
		strcpy(statusBuffer, "");
	}
}

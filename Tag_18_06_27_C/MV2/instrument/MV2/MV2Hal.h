// Name:
//	MV2Hal.h
//
// Purpose:
//	MV2 Hardware abstraction layer
//
// Description:
// Implement the communication between Arduino and MV2
//
// Coding Conventions:
//	- Variable names use the "InterCaps" convention
//	- Local variables are prefixed with an underscore, '_'
//	- Pointer variables are prefixed with 'p'
//	- Enumerated types are prefixed with 'e'
//	- Enumerated type values are prefixed with 'k'
//	- Constants are all uppercase
//	The above conventions can also be combined:
//	- A local pointer variable would be prefixed '_p'
//
//
// Change log:
//	22.01.16 SD	Original version
//	19.04.16 SD - Add ANALOG_OFFSET constant
//				- Move VDD_PIN to common section PIN
//				- Delete global variable
//	06.07.16 SD - Modify ANALOG_OFFSET constant according to the 10 bits resolution
//				- Add ANALOG_SHIFT constant
//				- Add Digital mode pin related to SPI
//				- Remove Interrupt Service Routine (ISR) for Data Ready (DR) signal. Genuino Uno doesn't trig on a high level signal.
//  31.03.17 PK - Add support for Arduino MEGA
//	21.08.17 PK - Rename ﻿DIGITAL_TO_ANALOG to NDIGITAL_ANALOG_PIN
//				- Define ﻿MV2_SPI_CLK_FREQ
//
// Copyright (c) 2016 Metrolab Technology SA, Geneva,
//	Switzerland

#ifndef MV2_HAL_H
#define MV2_HAL_H

#include "Arduino.h"
#include "MV2HostCommands.h"
#include "SPI.h"

// Magnetic field outputs are referenced to VCC/2 available on the REF pin
// The analog offset avoids a negative value which is not handled by host software
#define ANALOG_OFFSET			0x200

// Shift digitized analog value to a 16 bits format
#define ANALOG_SHIFT			6

// The conversion time depends on the resolution. 
// Maximum conversion time is with a 16 bits resolution. The refresh rate is 0.375KHz (3ms)
#define A_D_CONVERSION_TIMEOUT	5 // ms

/*
PIN CONFIGURATION

- Commnon PINs have no prefixed
- PINs used in digital mode are prefixed with D
- PINs used in analog mode are prefixed with A
*/

// COMMON
#define NDIGITAL_ANALOG_PIN     8
#define VDD_PIN					A5 

// DIGITAL MODE
#define MV2_SPI_CLK_FREQ        1000000
#define D_DR_PIN				2
#define D_INIT_PIN				7
#if defined(__AVR_ATmega328P__)     // UNO
    #define D_CHIP_SELECT_PIN       10
    #define D_SPI_MOSI				11
    #define D_SPI_MISO				12
    #define D_SPI_CLK               13
#elif defined(__AVR_ATmega2560__)   // MEGA 2560
    #define D_CHIP_SELECT_PIN       53
    #define D_SPI_MOSI              51
    #define D_SPI_MISO              50
    #define D_SPI_CLK               52
#else
    #error "Unknown board"
#endif

// ANALOG MODE
#define A_BX_PIN				A0
#define A_BY_PIN				A1
#define A_BZ_PIN				A2
#define A_TEMPERATURE_PIN		A3
#define A_REF_PIN				A4
#define A_EMR_PIN               2
#define A_INV_PIN				4
#define A_LP_PIN				7
#if defined(__AVR_ATmega328P__)     // UNO
    #define A_MA0_PIN				10
    #define A_MA1_PIN				11
    #define A_RA0_PIN				12
    #define A_RA1_PIN				13
#elif defined(__AVR_ATmega2560__)   // MEGA 2560
    #define A_MA0_PIN               53
    #define A_MA1_PIN               51
    #define A_RA0_PIN               50
    #define A_RA1_PIN               52
#else
    #error "Unknown board"
#endif

/*
ANALOG OPTIONS BITS
*/
#define B_RA0					0
#define B_RA1					1
#define B_MA0					2
#define B_MA1					3
#define B_LP					4
#define B_INV					5
#define B_EMR					6

/*
FORWARD DECLARATION for DIGITAL and ANALOG mode, MISCELLANEOUS
*/

// DIGITAL
eError			DigitalWaitForDataReady();									// Wait for Data Ready
eError			DigitalWriteAndRead(uint16_t Data, uint16_t *pReturnValue);	// Write value and read the previous selected data
uint8_t			DigitalReadRegister(uint8_t Register);						// Read register
void			DigitalSetInitBit(uint8_t Value);							// Set INIT bit

// ANALOG
uint16_t		AnalogDigitizeBx();											// Digitize Bx
uint16_t		AnalogDigitizeBy();											// Digitize By
uint16_t		AnalogDigitizeBz();											// Digitize Bz
uint16_t		AnalogDigitizeTemp();										// Digitize Temperature
void			AnalogSetOptions(uint8_t Options);							// Set OPTIONS bits (RA0, RA1, MA0, MA1, LP, INV, EMR)

// MISCELLANEOUS
void			MiscSetDigitalAnalogMode(eMode Mode);						// Set analog or digital mode
void			MiscConfigureAnalogModePin();								// Configure PIN for analoge mode
void			MiscConfigureDigitalModePin();								// Configure PIN for digital mode

// Return MV2 mode
eMode GetMV2Mode();

#endif // MV2_HAL_H

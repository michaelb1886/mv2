// Name:
//	MV2Hal.cpp
//
// Purpose:
//	MV2 Hardware abstraction layer
//
// Description:
// See MV2Hal.h
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
//	01.03.16 SD - Fix PIN mode for Data Ready PIN inMiscConfigureDigitalModePin
//				- Fix interrupt mode in MiscSetDigitalAnalogMode
//				- Now, MV2 mode is only updated in MiscSetDigitalAnalogMode
//	19.04.16 SD - Add comments
//				- Refactoring DigitalWaitForDataReady function
//				- Handle MV2 mode (Add GetMV2Mode function)
//	06.07.16 SD - Remove Interrupt Service Routine (ISR) for Data Ready (DR) signal. Genuino Uno doesn't trig on a high level signal.
//				- Fix DigitalWaitForDataReady according to this modification.
//				- Wait for Data Ready signal in DigitalWriteAndRead function
//				- In digital mode, set to default value analog inversion bit
//  16.03.17 PK - Do not wait for Data Ready signal in DigitalWriteAndRead function
//  24.03.17 PK - After INIT=1, toggle CS to clear spurious DR
//	21.08.17 PK - Clean up code: 
//				  - Use SPI.begin/endTransaction; 
//				  - Don't use detachInterrupt because was not attached
//				  - Call SPI.end only if in digital mode
//				  - Check analog/digital mode by reading configuration pin
//				  - Identify additional pin configurations common to analog/digital
//	22.08.17 ST - Fix MiscSetDigitalAnalogMode. Avoid initializing SPI multiple times
//  22.08.17 PK - More code cleanup: consolidate initialization of pins and their output values
//
// Copyright (c) 2016 Metrolab Technology SA, Geneva,
//	Switzerland

#define _GLOBAL_MV2_MODE_

#include "MV2Hal.h"

// MV2 mode
static enum {kUnconfigured, kConfiguredAnalog, kConfiguredDigital} _MV2Mode = kUnconfigured;

/*
	Return MV2 mode
*/
eMode GetMV2Mode()
{
	return (digitalRead(NDIGITAL_ANALOG_PIN) ? kAnalogMode : kDigitalMode);
}

/*
	DIGITAL function
	Write data (Register address, value) and read previously selected data value
	Parameters:
		[in]	Data : data to write
		[out]	pReturnValue : data received trough MISO
	Returns:
		eError : error
*/
eError DigitalWriteAndRead(uint16_t Data, uint16_t *pReturnValue)
{
	eError _Error = kNoError;

    // Begin the SPI transaction(s).
    SPI.beginTransaction(SPISettings(MV2_SPI_CLK_FREQ, MSBFIRST, SPI_MODE0));
	// Select chip
	digitalWrite(D_CHIP_SELECT_PIN, LOW);
	// Write _value and read previously selected data value
	*pReturnValue = SPI.transfer16(Data);
	// Unselect chip
	digitalWrite(D_CHIP_SELECT_PIN, HIGH);
    // Close the SPI transaction.
    SPI.endTransaction();
	
	return kNoError;
}

/*
	DIGITAL function
	Read register
	Parameters:
		[in]	Register : register to read
	Returns:
		uint8_t : register content
*/
uint8_t DigitalReadRegister(uint8_t Register)
{
	uint8_t _Value;

    // Begin the SPI transaction(s).
    SPI.beginTransaction(SPISettings(MV2_SPI_CLK_FREQ, MSBFIRST, SPI_MODE0));
	// Select chip
	digitalWrite(D_CHIP_SELECT_PIN, LOW);
	// read dummy, address=reg to read the content of register 
	SPI.transfer(Register);
	// read content of register 
	_Value = SPI.transfer(0);
	// Unselect chip
	digitalWrite(D_CHIP_SELECT_PIN, HIGH);
    // Close the SPI transaction.
    SPI.endTransaction();

	return _Value;
}

/*
	DIGITAL function
	Wait for Data Ready
	Parameters:

	Returns:
		eError
*/
eError DigitalWaitForDataReady()
{
	// Error
	eError _Error = kNoError;

	// Configure timeout
	unsigned long _TimeOut = millis() + A_D_CONVERSION_TIMEOUT;

	// Wait for end of conversion or timeout
	while (true) 
	{
		if (digitalRead(D_DR_PIN) == HIGH)
			break;
		else if (millis() >= _TimeOut)
		{
			_Error = kAdcTimeOutError;
			break;
		}
	}
	return _Error;
}

/*
	DIGITAL function
	Set INIT bit
	Parameters:
		[in]	Value : INIT bit value
	Returns:
		void
*/
void DigitalSetInitBit(uint8_t Value)
{
	digitalWrite(D_INIT_PIN, Value);

    // Toggle CS to clear spurious DR.
    if (Value)
    {
        digitalWrite(D_CHIP_SELECT_PIN, LOW);
        digitalWrite(D_CHIP_SELECT_PIN, HIGH);
    }
}

/*
	ANALOG function
	Digitize Bx, subtract the digitized REF value
	Parameters:

	Returns:
		uint16_t : Bx digitized
*/
uint16_t AnalogDigitizeBx()
{
	return ((analogRead(A_BX_PIN) - analogRead(A_REF_PIN)) + ANALOG_OFFSET) << ANALOG_SHIFT;
}

/*
	ANALOG function
	Digitize By, subtract the digitized REF value
	Parameters:

	Returns:
		uint16_t : By digitized
*/
uint16_t AnalogDigitizeBy()
{
	return ((analogRead(A_BY_PIN) - analogRead(A_REF_PIN)) + ANALOG_OFFSET) << ANALOG_SHIFT;
}

/*
	ANALOG function
		Digitize Bz, subtract the digitized REF value
	Parameters:

	Returns:
		uint16_t : Bz digitized
*/
uint16_t AnalogDigitizeBz()
{
	return ((analogRead(A_BZ_PIN) - analogRead(A_REF_PIN)) + ANALOG_OFFSET) << ANALOG_SHIFT;
}

/*
	ANALOG function
	Digitize temperature
	Parameters:
	
	Returns:
		uint16_t : temperature digitized
*/
uint16_t AnalogDigitizeTemp()
{
	return analogRead(A_TEMPERATURE_PIN) << ANALOG_SHIFT;
}

/*
	ANALOG function
	Set options
	Parameters:
		[in]	Options : Set options bits : RA0, RA1, MA0, MA1, LP, INV, EMR
	Returns:
		void
*/
void AnalogSetOptions(uint8_t Options)
{
	digitalWrite(A_RA0_PIN,	bitRead(Options, B_RA0));
	digitalWrite(A_RA1_PIN,	bitRead(Options, B_RA1));
	digitalWrite(A_MA0_PIN,	bitRead(Options, B_MA0));
	digitalWrite(A_MA1_PIN,	bitRead(Options, B_MA1));
	digitalWrite(A_LP_PIN,	bitRead(Options, B_LP));
	digitalWrite(A_INV_PIN, bitRead(Options, B_INV));
	digitalWrite(A_EMR_PIN, bitRead(Options, B_EMR));
}

/*
	MISCELLANEOUS function
	Set digital or analog mode
	Parameters:
		[in]	Mode : digital or analog mode
	Returns:
		void
*/
void MiscSetDigitalAnalogMode(eMode Mode)
{
	// According to the Mode, configure PIN direction, specific parameters
	switch (Mode)
	{
		case kDigitalMode:
			// If not already configured in this mode
			if (_MV2Mode != kConfiguredDigital)
			{
				// Configure digital mode pin
				MiscConfigureDigitalModePin();		
				//Start SPI
				SPI.begin();
                // Settling time
                delay(1);
				// Update variable _MV2Mode
				_MV2Mode = kConfiguredDigital;
			}
			break;
		case kAnalogMode:
			// If not already configured in this mode
			if (_MV2Mode != kConfiguredAnalog)
			{
				// Ensure SPI is disabled
		        if (_MV2Mode == kConfiguredDigital)
		            SPI.end();
				// Configure analog mode PIN
				MiscConfigureAnalogModePin();
				// Set analog reference to external
				analogReference(EXTERNAL);
                // Settling time
                delay(1);
				// Update variable _MV2Mode
				_MV2Mode = kConfiguredAnalog;
			}
			break;
	}
}

/*
	MISCELLANEOUS function
	Configure PIN common to analog and digital mode
	Parameters:

	Returns:
		void
*/

void MiscConfigureCommonPin()
{
	pinMode(VDD_PIN, INPUT);
    pinMode(A_BX_PIN, INPUT);
    pinMode(A_BY_PIN, INPUT);
    pinMode(A_BZ_PIN, INPUT);
    pinMode(A_TEMPERATURE_PIN, INPUT);
    pinMode(A_REF_PIN, INPUT);
    pinMode(A_INV_PIN, OUTPUT);

    // Initialize INV output to LOW
    digitalWrite(A_INV_PIN, LOW);

}

/*
	MISCELLANEOUS function
	Configure Analog mode PIN
	Parameters:

	Returns:
		void
*/
void MiscConfigureAnalogModePin()
{
	// Set common PIN
	MiscConfigureCommonPin();

	// Set PINs used in analog mode
    pinMode(NDIGITAL_ANALOG_PIN, OUTPUT);
	pinMode(A_MA0_PIN, OUTPUT);
	pinMode(A_MA1_PIN, OUTPUT);
	pinMode(A_RA0_PIN, OUTPUT);
	pinMode(A_RA1_PIN, OUTPUT);
	pinMode(A_LP_PIN, OUTPUT);
	pinMode(A_EMR_PIN, OUTPUT);

    // Ananlog/digital selector must be HIGH
    digitalWrite(NDIGITAL_ANALOG_PIN, HIGH);

    // Initialize all other controls to LOW (logic 0)
    digitalWrite(A_MA0_PIN, LOW);
    digitalWrite(A_MA1_PIN, LOW);
    digitalWrite(A_RA0_PIN, LOW);
    digitalWrite(A_RA1_PIN, LOW);
    digitalWrite(A_LP_PIN, LOW);
    digitalWrite(A_EMR_PIN, LOW);
}

/*
    MISCELLANEOUS function
    Configure Digital mode PIN
    Parameters:

    Returns:
        void
*/
void MiscConfigureDigitalModePin()
{
    // Set common PIN
    MiscConfigureCommonPin();

    // Set PINs used in digital mode
    pinMode(NDIGITAL_ANALOG_PIN, OUTPUT);
    pinMode(D_DR_PIN, INPUT);
    pinMode(D_INIT_PIN, OUTPUT);
    pinMode(D_CHIP_SELECT_PIN, OUTPUT);

    // Ananlog/digital selector must be LOW
    digitalWrite(NDIGITAL_ANALOG_PIN, LOW);
    
    // MagVector Chip Select (CS) must be deactived
    digitalWrite(D_CHIP_SELECT_PIN, HIGH);
    
    // Initialize INIT to low
    digitalWrite(D_INIT_PIN, LOW);
}

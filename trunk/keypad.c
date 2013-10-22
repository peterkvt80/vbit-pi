/*
             KEYPAD CONTROL
     Copyright (C) MRG Systems Ltd, 2012.
              
        peter@mrgsystems.co.uk
         www.mrgsystems.co.uk
*/
/* keypad.c Routines to read an MRG keypad.
 * The keypad should be run on 3.3V instead of 5V in order to work with the Raspberry Pi
 * The keypad power should be connected to the VBIT-Pi onboard 3V3 LDO regulator.
 */

#include "keypad.h"
#define SLAVE_ADDRESS_MRGKEYPAD 0x20
// TODO: Probably need 8 file descriptors for all the possible keypad addresses
static int fd[8];	// File descriptor 

uint8_t KeypadError[MAXKEYPAD];
uint8_t KeypadSuspend[MAXKEYPAD];

uint32_t keypad_init(void)
{
	uint8_t i;
	for (i=0;i<8;i++)
		fd[i]=wiringPiI2CSetup (SLAVE_ADDRESS_MRGKEYPAD+i*2);
	return fd[0];
} // keypad_init


/* Send a single value to the PCF I/O chip */
/************************************************************************/
/* \return 0 if OK, else consult errno                                  */
/************************************************************************/
uint8_t write_keypad(uint8_t address, uint8_t data)
{
	// TODO: Use address to select an array of fd[address]
	return wiringPiI2CWrite (fd[address],data);
}

/* Read a single value from the PCF I/O chip */
uint8_t read_keypad(uint8_t address)
{
	uint8_t result;
	result=wiringPiI2CRead(fd[address]);
	return result;
} // read_keypad

/**
 * read_keychar
 * \param addr - keypad unit address 0..7 (as set on the hex key)
 * \return - 0 if failed or a valid unsigned char 0x30 - 0x3f.
 * lowest two bits 1:0 are column, starting from the right.
 * next two bits 3:2 are row, starting from the top
 *   _________ So the keypad returns 0x30 and
 *   |3|2|1|0|
 *   ---------
 *   |7|6|5|4|
 *   ---------
 *   |b|a|9|8|
 *   ---------
 *   |f|e|d|c|
 *   ---------
 * Each keypad is different so you need to map this accordingly.
 *
 * Rules:
 * A valid key is returned only once.
 * A key is returned on the press of a key.
 * There will be no further valid keys until after ALL the keys are released
 * There is a 10ms delay before a valid key is returned, 5ms if the row is invalid
 * If the initial press is more than one key, then no more presses will be sent until AFTER all the keys are released.
 * 
 * 
 */
uint8_t read_keychar(uint8_t addr)
{
	static uint8_t released[8]={true,true,true,true,true,true,true,true};
	static uint8_t errorcode[8]={0,0,0,0,0,0,0,0};
	uint8_t result;
	uint8_t row;
	uint8_t col;
	// Get the 4 bits for the row
	result=write_keypad(addr,0x0f);
	delay(1);

	KeypadError[addr]=result>0;
	if (result>0) KeypadSuspend[addr]=0xff; // How many loops to skip before polling again		
	
	// Some sort of I2C error
	if (result)
	{
		errorcode[addr]=0x80|result;
		return errorcode[addr];	// Set bit 7 to indicate an error
	}	
	else
		errorcode[addr]=0;
		
	row=0;
	row=(~read_keypad(addr)) & 0x0f;		
	
	// Check the row
	switch (row)
	{
		case 0x01 : row=0;break;
		case 0x02 : row=1;break;
		case 0x04 : row=2;break;
		case 0x08 : row=3;break;
		case 0x00 : released[addr]=true;
					return 0;
		default:
			released[addr]=false;	// zero-key rollover
			return 0;
	}
	// Get the 4 bits for the column
	result=write_keypad(addr,0xf0);delay(1);
	if (result)
		return 0x80|result;	// If bit 7 is set, it is an error
	col=0;

	col=((~read_keypad(addr))>>4) & 0x0f;
	result=write_keypad(addr,0x00);
	
	if (result)
		return 0x80|result;	// If bit 7 is set, it is an error
	
	// Check the column
	switch (col)
	{
		case 0x01 : col=0;break;
		case 0x02 : col=1;break;
		case 0x04 : col=2;break;
		case 0x08 : col=3;break;
		case 0x00 : released[addr]=true;
					return 0;
		default:
			released[addr]=false;	// zero-key rollover
			return 0;
	}
	if (released[addr])
	{
		// YAY! All is fine.
		released[addr]=false;
		return 0x30|col<<2|row;		// 0x30 to 0x3f
	}
	else
		return 0;	// Still waiting to release the last key
} // read_keychar

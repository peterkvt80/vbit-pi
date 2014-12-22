/** ***************************************************************************
 * Description       : VBIT: FIFO Control
 * The FIFO uses SPI. The clock is multiplexed so that control can 
 * be shared between the CPU and the video encoder (DENC)
 *
 * Compiler          : GCC
 *
 * Copyright (C) 2013, Peter Kwan
 *
 * Permission to use, copy, modify, and distribute this software
 * and its documentation for any purpose and without fee is hereby
 * granted, provided that the above copyright notice appear in all
 * copies and that both that the copyright notice and this
 * permission notice and warranty disclaimer appear in supporting
 * documentation, and that the name of the author not be used in
 * advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 *
 * The author disclaims all warranties with regard to this
 * software, including all implied warranties of merchantability
 * and fitness.  In no event shall the author be liable for any
 * special, indirect or consequential damages or any damages
 * whatsoever resulting from loss of use, data or profits, whether
 * in an action of contract, negligence or other tortious action,
 * arising out of or in connection with the use or performance of
 * this software.
 ****************************************************************************/
#include "fifo.h"
 
/*
Oh dear. The drivers insist on de-asserting the chip select. Use plan B
Use a GPIO line as the chip select.
*/

static int fdspi=-1;	// File descriptor for the SPI port

int spiram_initialise(void)
{
	/* Channel 0 of SPI. We choose a really slow SPI clock because: we don't need it to go fast
	and also fast speeds are likely to get mangled in the ribbon cable 	
	500000 is the slowest mentioned in the documentation */
	digitalWrite(GPIO_CSN, HIGH);	
	fdspi=wiringPiSPISetup (0, 2000000);
	return fdspi;
}

void SetSerialRamStatus(unsigned char status)
{
	uint8_t buf[2];
	// AVR XMEGA version
	// TBA set the mux here too!
	// VBIT Mux needs setting as SCK is shared
	//GPIO_On(VBIT_SEL);	
	/* MASTER: Pull SS line low. This has to be done since
	 *         SPI_MasterTransceiveByte() does not control the SS line(s). */
	//SPI_MasterSSHigh(ssPort, PIN4_bm); // Toggle the chip to reset any mode
	//SPI_MasterSSLow(ssPort, PIN4_bm);
	//SPI_MasterTransceiveByte(&spiMaster,SPIRAM_WRSR); // Write status register
	//SPI_MasterTransceiveByte(&spiMaster,status); // Write status register
	//SPI_MasterSSHigh(ssPort, PIN4_bm); // Release the RAM
	// Raspberry PI version
	digitalWrite(GPIO_CSN, HIGH);	
	delayMicroseconds(1);
	digitalWrite(GPIO_CSN, LOW);	
	buf[0]=SPIRAM_WRSR;
	buf[1]=status;
	wiringPiSPIDataRW(0, buf, 2);
	digitalWrite(GPIO_CSN, HIGH);	
}

int GetSerialRamStatus(void)
{
	uint8_t buf[2];
	// TBA set the mux here too!
	/* MASTER: Pull SS line low. This has to be done since
	 *         SPI_MasterTransceiveByte() does not control the SS line(s). */
	//SPI_MasterSSHigh(ssPort, PIN4_bm); // Toggle the chip to reset any mode
	//SPI_MasterSSLow(ssPort, PIN4_bm);
	//SPI_MasterTransceiveByte(&spiMaster,SPIRAM_RDSR); // Write status register
	//result=SPI_MasterTransceiveByte(&spiMaster,'?'); // Write status register
	//SPI_MasterSSHigh(ssPort, PIN4_bm); // Release the RAM
	// Raspberry Pi version
	digitalWrite(GPIO_CSN, HIGH);	// Deselect and reselect
	delayMicroseconds(1);
	digitalWrite(GPIO_CSN, LOW);	
	buf[0]=SPIRAM_RDSR;
	buf[1]='?';
	wiringPiSPIDataRW(0, buf, 2);	// Transact
	digitalWrite(GPIO_CSN, HIGH);	
	return buf[1];
}

void SetSerialRamAddress(unsigned char RWMode, uint16_t address)
{
	//delay_us(1); // Was 1. 
	//SPI_MasterSSHigh(ssPort, PIN4_bm); // Toggle the chip to reset any mode
	//delay_us(1); // Was 1. 
	//SPI_MasterSSLow(ssPort, PIN4_bm);
	//delay_us(1); // Was 1. Can set it back if this doesn't help
	uint8_t buf[3];
	digitalWrite(GPIO_CSN, HIGH);	// Deselect and reselect
	delayMicroseconds(1);
	digitalWrite(GPIO_CSN, LOW);	
	buf[0]=RWMode;
	buf[1]=(address>>8)&0xff;
	buf[2]=address&0xff;
	wiringPiSPIDataRW(0, buf, 3);	// Transact then hold the chip because we are going to do a data transact
	
	//SPI_MasterTransceiveByte(&spiMaster,RWMode); // Read or Write command
	// now the 15 address bits
	//SPI_MasterTransceiveByte(&spiMaster,(address>>8)&0xff); // Write address high
	//SPI_MasterTransceiveByte(&spiMaster,address&0xff); // Write address low
}

// Why should I set
void WriteSerialRam(char *buffer, int length, int addr)
{
	uint16_t i;
	uint8_t buf[2048];
	digitalWrite(GPIO_CSN, HIGH);	
	delayMicroseconds(1);
	digitalWrite(GPIO_CSN, LOW);	
	// Put the address in
	buf[0]=SPIRAM_WRITE;
	buf[1]=(addr>>8)&0xff;
	buf[2]=addr&0xff;
	// Copy the rest of the data
	for (i=0;i<length;i++)
		buf[i+3]=buffer[i];
	wiringPiSPIDataRW(0, buf, length+3);	// transaact to device 0
	// After a write we should deselect
	digitalWrite(GPIO_CSN, HIGH);
}

void ReadSerialRam(char *buffer, int length, int addr)
{
	uint16_t i;
	uint8_t buf[2048];
	digitalWrite(GPIO_CSN, HIGH);	// Deselect and reselect
	delayMicroseconds(1);
	digitalWrite(GPIO_CSN, LOW);
	buf[0]=SPIRAM_READ;
	buf[1]=(addr>>8)&0xff;
	buf[2]=addr&0xff;
	// Copy the rest of the data 
	for (i=0;i<length;i++)
		buf[i+3]=buffer[i];
	wiringPiSPIDataRW(0, buf, length+3);	// transact
	// return the buffer
	for (i=0;i<length;i++)
		buffer[i]=buf[i+3];
	buffer[length]=0;
	// Do not deselect the chip as we are probably going to let VBIT clock it.
	// If you do a read from the host then after you'll have to deselectserialram.
}

void DeselectSerialRam(void)
{
	//SPI_MasterSSHigh(ssPort, PIN4_bm); // Release the last control command.
	digitalWrite(GPIO_CSN, HIGH);	
}
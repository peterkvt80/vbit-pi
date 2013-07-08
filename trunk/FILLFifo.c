/** FIFOFill
 * What is this? It is the thread that copies a field of VBI to the FIFO.
 * Copyright Peter Kwan (c) 2013. All rights reserved
 */

#include "FILLFifo.h"

// A test routine to stuff a buffer full of text

/** diff
 * Pinched from Guy Rutenberg
 * \param start : start time specification structure
 * \param end : end time
 * \return timespec of the end-start times
 */
struct timespec diff(struct timespec start, struct timespec end)
{
	struct timespec temp;
	if ((end.tv_nsec-start.tv_nsec)<0) {
		temp.tv_sec = end.tv_sec-start.tv_sec-1;
		temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
	} else {
		temp.tv_sec = end.tv_sec-start.tv_sec;
		temp.tv_nsec = end.tv_nsec-start.tv_nsec;
	}
	return temp;
} // diff

/** FIFOFill
 * Thread that copies from the VBI buffer to the FIFO.
 */
PI_THREAD (FillFIFO)
{
	//int result;
	struct timespec rightnow;	
	struct timespec sincevbi;
	long ms,ms2;
	int i;
	uint8_t ch='a';
	char mydata[45*17];	// This will be replaced by the global VBI buffer 
	char caption[100];
	strcpy(caption,"Raspberry PiFAX   ");
	while(1)
	{
		// printf(">");
		// Wait for the VBI to happen then reset the VBIStarted flag
		while (gVBIStarted==LOW);	// Probably should signal a video failure instead of getting stuck
		gVBIStarted=LOW;	// OK, we got the trigger, reset and carry on
		// What is the time now?
		/*result=*/clock_gettime(CLOCK_REALTIME,&rightnow);	// some sort of Linux timestamp		
		// Elapsed time since the field interrupt
		sincevbi=diff(gFieldTimespec,rightnow);
		ms=sincevbi.tv_nsec/1000;	// Ignore seconds, just get the milliseconds
		// Suspend the thread until the VBI is done.
		// By definition PAL is 625 lines of which 576 are active.
		// So VBI is 49 lines and per field we will round it up to 25 lines.
		// VBI delay should be at least 64us x 25 = 1600us.
		if (ms<1600)
			delayMicroseconds(1600-ms); 
		// We should be past the VBI now.
		// Set up I/O ready to access the serial ram 
		DeselectSerialRam();			// this stops read mode  
		digitalWrite (GPIO_MUX, LOW) ;	// mux to the CPU clock
		// Fill mydata with text packets that we want to send
		ch++;
		if (ch>'z') ch='a';
		caption[17]=ch;	// Add a little beacon
		for (i=0;i<16;i++) // TODO. Odd and Even must be accounted for. Check on scope
		{	
			// TODO: Check if we have a buffer under-run
			if (bufferGet(streamBuffer,&mydata[i*PACKETSIZE]))
			{
				//printf("[FillFIFO] bufferGet failed\n");
			}
			//else
			//	printf("[FillFIFO] GOT STUFF !!!!!!!! bufferGet \n");
			// WriteSerialRam(&mydata[0],PACKETSIZE*17,0);				// Write buffer to FIFO address 0
		}
		/*
		{
		// Perhaps add a few rows of data so we can see if anything is working
			PacketHeader(&mydata[(i+i)*PACKETSIZE],i,0,0,0x8040,caption);
			PacketPrefixValue(&mydata[(i+i+1)*PACKETSIZE],i,i+1,i+'A');
			Parity(&mydata[(i+i+1)*PACKETSIZE],5);
			// PacketClear(&mydata[(i+8)*PACKETSIZE],0xff);		
		}
		*/
//*packet ,unsigned char mag, unsigned char page, unsigned int subcode,
			//unsigned int control, char *caption);		
		WriteSerialRam(&mydata[0],PACKETSIZE*17,0);				// Write buffer to FIFO address 0

		// Get the source VBI address
		// Get the FIFO address (probably always 0)
		// Grab the FIFO using CSN
		// Send a field worth of data to the FIFO
		// Set the FIFO up for VBIT
		SetSerialRamAddress(SPIRAM_READ,0); // Set up a read from address 0. Chip stays selected.
		digitalWrite (GPIO_MUX, HIGH) ;	// mux to the CPU clock
		// Calculate how much time we need to sleep and do it		
		clock_gettime(CLOCK_REALTIME,&rightnow);	// Time now
		sincevbi=diff(gFieldTimespec,rightnow);		// Time since VBI
		ms2=20000-sincevbi.tv_nsec/1000;	// ...in microseconds
		delayMicroseconds(ms2);	// This is critical. Check with an oscilloscope.
		// T1 is the time which it took us to notioe the VBI
		// T2 is the sleep time we can wait until the next VBI.
		// printf("T1=%d T2=%d\n",ms,ms2);
	}
} // FIFOFill



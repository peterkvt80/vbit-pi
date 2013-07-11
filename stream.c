/** Stream.c ************************
 * Thread that creates a stream of packets 
 * Most packets are taken from the mazagine threads
 * Other packets are generated as needed:
 * Packet 8/30, subtitles, databroadcast
 * Packets are sequenced by mag priority, primary and secondary actions.
 * They also interact with the mag state engines to ensure that headers
 * and their rows don't appear on the same field.
 * Sources packets from mag.c
 * Sinks packets to FillFIFO.c 
 * Copyright (c) 2013 Peter Kwan. All rights reserved.
 */
#include "stream.h"

// The stream buffer is 50 packets

bufferpacket streamBuffer[STREAMBUFFERSIZE];
uint8_t streamPacket[STREAMBUFFERSIZE*PACKETSIZE];

static char priority[8]={5,1,1,3,3,5,5,9};
static char priorityCount[8];

// TODO Is the current packet. Implemented elsewhere
uint8_t isHeader(bufferpacket *bp)
{
return 0;
}

PI_THREAD (Stream)
{
	int mag=0;
	uint8_t line=0;
	uint8_t i;
	uint8_t result;
	uint8_t hold[8];	// If hold is set then we must wait for the next field to reset them
	for (i=0;i<8;i++)
	{
		hold[i]=0;
		priorityCount[i]=priority[i];
	}
	// Initialise the stream buffer
	bufferInit(streamBuffer,(char*)streamPacket,STREAMBUFFERSIZE);
	delay(3000);	// Give the other threads a chance to start sending packets.
	while(1)
	{
		// printf("Hello from stream\n");
		// delayMicroseconds(100);

		// Decide which mag can go next
		for (;priorityCount[mag]>0;mag=(mag+1)%8)
		{
			priorityCount[mag]--;
		}
		//printf(" M%d ",mag);

		// This scheme more or less works but there is no guarantee that it stays in sync.
		// So we need to ensure that FillFIFO knows what this phase is and matches it to the output.
		// The 7120 DENC only does up to 16 lines on both fields. Line 17 is not available for us :-(
		if (line<16)
		{
			line++;						
		}
		else
		{
			line=0;
			for (i=0;i<8;i++) hold[i]=0;	// Any holds are released now
		}	
		// If the source has data AND the destination has space AND the magazine has not just sent an header
		if (!hold[mag])
		{
			// Pop a packet from a mag and push it to the stream
			// printf("[Stream] ******* GOT SOMETHING :-) Get mag buffer %d \n",mag);
			// dumpPacket(magBuffer[mag].pkt);
			result=bufferMove(streamBuffer,&(magBuffer[mag]));

			switch (result)
			{
			case 3: 	// Buffer full. This is good because it means that we are not holding things up
				// printf("[Stream] Destination full\n");// Either we are up to date or FIFOFill has stalled. Which one is it? 
				//delay(1);	// Hold so we can see the message (WiringPi)
				break;
			case 4: 	// Source not ready. We expect mag to send us something very soon
				// If a stream has no pages, this branch will get called a lot
				// printf("[Stream] Waiting for source\n"); // If this is called then mag has failed.
				delay(1);	// Hold so we can see the error
				break;
			case 2:		// Header row
				// printf("%01d",mag);
				hold[mag]=1;
				// Intentional fall through
			case 0: break; // Normal row
			}
			// TODO: If ALL fields are on hold we need to output filler or quiet
			// TODO: Implement stream side buffer
		}
		priorityCount[mag]=priority[mag];	// Reset the priority for the mag that just went out
		//else
		//	printf("[Stream] mag buffer %d is empty\n",mag);
	}
} 

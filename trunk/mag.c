/** ***************************************************************************
 * Description       : VBIT: Magazine page to packet converter.
 * The Pages folder is scanned for pages belonging to a mag.
 * This list of pages is used to sequence packets for this mag.
 * There are eight instances of this thread, one per mag.
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
/** Mag thread
 * Each magazine operates in its own thread.
 * Each mag knows what mag number it is.
 * Each mag has a state. IDLE/HEADER/ROW to help sequence packets
 * Each mag buffers about 50 packets ahead.
 */

#include "mag.h"
// Number of packets in a magazine buffer. 17 is an arbitrary number
#define PACKETCOUNT 17

int r1=0, r2=0;

uint8_t thismag;

bufferpacket magBuffer[8];	// One buffer control block for each magazine


uint8_t magPacket[8][PACKETCOUNT][PACKETSIZE];	// 8 threads, 17 packets, 45 bytes per packet  

static pthread_t magThread[8];

static uint8_t magCount=1;	// Ensure that each thread has a different mag number

/** getMag - Allocate a magazine to a thread.
 */
uint8_t getMag(void)
{
	uint8_t num;	
	piLock(0);	// Ensure that each mag is unique
	num=magCount;
	magCount=(magCount+1)%8;
	piUnlock(0);
	return num;
}

/** getList - Populate a magazine list
 * Declare the list in domag so we use auto variables. So each thread gets its own environment.
 */
uint8_t getList(PAGE **txList,uint8_t mag)
{
  DIR *d;		// Directory handle
  PAGE page;
  PAGE *p;		// Page that we are going to parse
  PAGE *newpage;
  char path[132];
  char filename[132];
  struct dirent *dir;
  strcpy(path,"/home/pi/Pages/");
  //printf("Looking for pages in stream %d\n",mag);
  d = opendir(path);
  p=&page;
  if (d)
  {
    while ((dir = readdir(d)) != NULL)
    {
	  // TODO: Is it a directory?
	  // Is it a tti page
	  if (strcasestr(dir->d_name,".tti"))
	  {
		  strcpy(filename,path);
		  strcat(filename, dir->d_name);
		  // printf("stream %d, %s\n", mag, filename);
		  if (ParsePage(p, filename))
		  {
				printf("Not a valid page\n");
		  }
		  // printf("Comparing p->mag %d, p->page %d, mag %d\n",p->mag % 8, p->page, mag);
		  if ((p->mag % 8)==mag)
		  {
			strcpy(p->filename,filename);
			// printf("Accepted page %s\n",p->filename);
			// Create a new page object
			newpage=calloc(1,sizeof(PAGE));
			// Copy the data
			*newpage=*p;
			// And drop the pointer into the transmission list
			// Check that we don't have a duplicate!!!
			if (txList[p->page])
			{
				printf("[mag:getList] Page already exists. old=%s, new=%s\n",txList[p->page]->filename,p->filename);
			}
			txList[p->page]=newpage;
			// printf("[getList]Saved page %s mpp=%01d%02d\n",txList[p->page]->filename,txList[p->page]->mag,txList[p->page]->subpage);
		  }
	  }
	  // TODO: Something wonderful with the PAGE object
    }

    closedir(d);
  }
  else
	return 1;

  return 0;	
} // getList

/** domag is the thread that manages a single magazine
 * Finds and sorts all the pages for a particular magazine.
 * Manages adding and removing pages.
 * Maintains a state machine: IDLE, HEADER, ROW
 */
void domag(void)
{
	char blah[32];
	uint16_t i;
	uint8_t txListIndex;	// The current page
	uint8_t txListStart;	// Help prevent an endless loop
	PAGE* txList[256];	// One pointer per page. There are 256 possible pages in a magazine
	uint8_t mag;
	uint8_t row;
	uint8_t state;
	uint8_t packet[PACKETSIZE];
	PAGE* page;
	FILE* fil;
	char str[80];
	// Init the transmission list for this magaine
	for (i=0;i<256;i++)
		txList[i]=NULL;
	// Work out which magazine we are
	mag=getMag();
	// Find the pages for this mag and put them into the transmission list.
	piLock(1);
	if (getList(txList,mag))
	{
		// printf("Could not find pages on stream %d\n",mag);
		piUnlock(1);
		return;
	}
	piUnlock(1);
	printf("Mag thread is initialised: mag=%d\n",mag);
	
	// Initialise the magazine state
	state=STATE_BEGIN;
	// Start at page 0
	txListIndex=0;
	// The mag loop has a page counter that steps through all the pages
	// There is a state variable which helps step through the file.
	while(1)
	{
		// printf("Here is a mag thread, mag=%d\n",mag);
		while (bufferIsFull(&magBuffer[mag])) // If the buffer is full, just idle for a bit
		{
			// printf("mag is full, mag=%d\n",mag);
			delay(20); // ms
		}
		switch (state)
		{
		case STATE_BEGIN:	// First time only
			// do stuff the first time
			state=STATE_IDLE;
			break;
		case STATE_IDLE:	// Ready to start a new page
			// Find the next page to transmit
			txListStart=txListIndex;	// Avoid infinite loop if we have no pages in mag, (should go to another idle state if this happens)
			txListIndex++;
			while(!txList[txListIndex] )
			{
				txListIndex++;
				if (txListStart==txListIndex)	// oops. We got nothing
				{
					state=STATE_BEGIN;
					// printf("[domag] Magazine %d contains no pages\n",mag);
					delay(1000);	// Might as well do nothing most of the time
					break;
				}
			}
			//printf("[domag] selected file=%s\n",txList[txListIndex]->filename);
			// Now we have the page we are going to transmit. We can send the header and change state
			// TODO
			// Something like:
			// Get the page object
			page=txList[txListIndex];
			// printf("[domag] selected file=%s\n",txList[txListIndex]->filename);
			//printf("[domag] selected file=%s\n",page->filename);
			// printf("%c%01d%02x",mag+'a'-1,page->mag,page->page);
			// TX the header
			sprintf(blah,"P%01d%02x %s",page->mag,page->page,page->filename);
			// Create the header. Note that we force parallel transmission by ensuring that C11 is clear
			PacketHeader((char*)packet,page->mag,page->page,page->subcode,page->control & ~0x0040,blah);
			// The packet isn't quite finished. stream.c intercepts headers and adds dynamic elements, page, date, network ID etc.
			
			// PacketHeader(packet,mag,page->subpage,0,0x8040,"blah");
			
			if (bufferPut(&magBuffer[mag],(char*)packet))
				delay(20); // milliseconds
			// dumpPacket((char*)packet);
			// Open the page file
			// fil=fopen(page->filename,"r");

			// scan down to the rows
			str[0]=0;
			fil=fopen(page->filename,"r");
			while (strncmp(str,"OL,",3) && !feof(fil))
				fgets(str,80,fil);

			if (feof(fil))	// Not found any lines
			{
				fclose(fil);
				state=STATE_IDLE;
			}
			else	// This is the first output line. Parse it and process it.
			{
				// printf("First line is %s\n",str);
				row=copyOL((char*)packet,str);
				PacketPrefix((uint8_t*)packet, page->mag, row);
				Parity((char*)packet,5);				
				//dumpPacket(packet);
				while (bufferIsFull(&magBuffer[mag])) delay(20);				
				bufferPut(&magBuffer[mag],(char*)packet);	// TODO: Test for buffer full
				
				state=STATE_SENDING;
			}
			// Change to sending.
			// Set some sort of shared variable which says to pause because we are on the header.
			break;
		case STATE_HEADER:	// Doing the header (NOT ACTUALLY USED!)
			// Send the header according to transmission rule: After header, rows can start on next field.
	
			// This probably should be a meta-packet that starts with anything that is not CRI.
			// stream.c will use this to halt the mag until the next field.
			state=STATE_SENDING;
			break;
		case STATE_SENDING:	// Transmitting rows
			fgets(str,80,fil);
			// printf("[domag] Send a row %s\n",str);
			if (str[0]=='O' && str[1]=='L')	// Double check it is OL. It could be FL.
			{
				row=copyOL((char*)packet,str);
				PacketPrefix(packet,page->mag,row);			
				Parity((char*)packet,5);
				while (bufferIsFull(&magBuffer[mag])) delay(20);				
				bufferPut(&magBuffer[mag],(char*)packet);	// TODO: Test for buffer full
			}
			// When we run out of rows to send
			if (feof(fil))
			{
				state=STATE_IDLE;
				fclose(fil);	// TODO: Don't try to close already closed!
			}
			break;			
		}
		delay(1);	// Just something to break up the sequence
		// TODO: We should really intercept a shutdown and release all the memory
	}
} // domag


/** MagInit creates eight domag threads
 * It also sets up the buffers that each thread will use to forward packets.
 */
void magInit(void)
{
	int i;
	const int maxThreads=8; 	// should be 8
	magCount=1;
	for (i=0;i<8;i++)
	{
		// Set up the buffers, one per thread
		bufferInit(&magBuffer[i],(char*)&magPacket[i],PACKETCOUNT);
		// now got to add the packet data itself
	}
	for (i=0;i<maxThreads;i++) {
		magThread[(i+1)%8]=0;
		pthread_create(&magThread[(i+1)%8],NULL,(void*)domag,(void*)&r1);
	}
} // magInit



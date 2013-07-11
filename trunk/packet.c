/** packet.c
 * Packet handles teletext packets.
 * The basic transmission unit is 45 bytes.
 * These functions are used to set up the various packet types including:
 * header
 * row
 * quiet
 * filler
 * packet 8/30
 * Fastext
 *
 * Copyright (c) 2010-2013 Peter Kwan
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * The name(s) of the above copyright holders shall not be used in
 * advertising or otherwise to promote the sale, use or other
 * dealings in this Software without prior written authorization.
 *
 *****************************************************************************/ 
#include "packet.h"

void dumpPacket(char* packet)
{
	int i;
	for (i=0;i<45;i++)
		printf("%2x ",packet[i]); 
	printf("\n");
}

/** Copy a line of teletext in MRG tti format.
 * OL,nn,<line> 
 * Where nn is a line number 1..27
 * <line> has two methods of escaping, which need to be decoded
 * \return The row number
 */
uint8_t copyOL(char *packet, char *textline)
{
	int i;
	char* p;
	long linenumber;
	// Get the line number
	textline+=3;
	char ch;
	linenumber=strtol(textline,NULL,0);	
	// xatoi(&textline, &linenumber);
	//xprintf(PSTR("Line number=%d\n"),(int)linenumber);
	// Skip to the comma to get the body of the command
	for (i=0;i<4 && ((*textline++)!=',');i++);
	if (*(textline-1)!=',')
	{
		printf("[copyOL]Fail=%s",textline);
		return 0xff; // failed
	}
	for (p=packet+5;p<(packet+PACKETSIZE);p++)*p='P'; // If you see a line full of P, we dun goofed.
	for (p=packet+5;*textline && p<(packet+PACKETSIZE);textline++) // Stop on end of file OR packet over run
	{
		// TODO: Also need to check viewdata escapes
		// Only handle MRG mapping atm
		ch=*textline; // Don't strip off the top bit just yet!
		if ((ch!=0x0d) && (ch & 0x7f)) // Do not include \r or null
		{
			if ((ch & 0x7f)==0x0a)		
			{
				// *p=0x0d; // Translate lf to cr (double height)
				*p='?';
			}
			else
				*p=ch;
		}
		else
		{
			// \r is not valid in an MRG page, so it must be a truncated line
			// Fill the rest of the line in with blanks
			char *r;
			for (r=p;r<(packet+PACKETSIZE);r++)
				*r=' '; // fill to end with blanks
				// *r='z'-(char)(PACKETSIZE-(char)((int)packet+(int)r)); // was space but I want it visible
			// *r='E';    // Make the last one visibly different (debugging)
			break;
		}
		// if ((*p & 0x7f)==0) *p=' '; // In case a null sneaked in
		p++;
	}
	return linenumber;
} // copyOL


/** Check that parity and reverse bits
 * Offset should be 5 for rows, 13 for header)
 */
void Parity(char *packet, uint8_t offset)
{
	int i;
	uint8_t c;
	for (i=offset;i<PACKETSIZE;i++)
	{
		
		packet[i]=ParTab[(uint8_t)(packet[i]&0x7f)]; // Strange syntax because of ParTab in progmem
	}
	for (i=0;i<PACKETSIZE;i++)
	{
		c=(uint8_t)packet[i];
		c = (c & 0x0F) << 4 | (c & 0xF0) >> 4;
		c = (c & 0x33) << 2 | (c & 0xCC) >> 2;
		c = (c & 0x55) << 1 | (c & 0xAA) >> 1;	
		packet[i]=(char)c;
	}
} // Parity

// Clear entire packet to a value
void PacketClear(uint8_t *packet, uint8_t value)
{
	uint8_t i;
	for (i=0;i<PACKETSIZE;i++)
		packet[i]=value;
}

// Clear enture packet to 0
void PacketQuiet(uint8_t *packet)
{
	PacketClear(packet,0);
}

// Set CRI and MRAG. Set rest of packet to "value"
void PacketPrefixValue(uint8_t *packet, uint8_t mag, uint8_t row, int value)
{
	char *p=(char*)packet; // Remember that the bit order gets reversed later
	if (value<256) PacketClear(packet,value);	// For debugging this helps
	*p++=0x55; // clock run in
	*p++=0x55; // clock run in
	*p++=0x27; // framing code
	// add MRAG
	*p++=HamTab[mag%8+((row%2)<<3)]; // mag + bit 3 is the lowest bit of row
	*p++=HamTab[((row>>1)&0x0f)];
} // PacketPrefix

// Set CRI and MRAG and ignore everything else
void PacketPrefix(uint8_t *packet, uint8_t mag, uint8_t row)
{
	PacketPrefixValue(packet,mag,row,999); // 999=Don't mess up the rest of the packet!
}

// Make a filler packet
void PacketFiller(uint8_t *packet)
{
	PacketPrefixValue(packet,8,25,' ');
}

/** A header has mag, row=0, page, flags, caption and time
 */
void PacketHeader(char *packet ,unsigned char mag, unsigned char page, unsigned int subcode,
			unsigned int control, char *caption)
{
	char *p;
	char ch;
	int i,j;
	//static int lastsec;
	
	
	// strcpy(caption,"Hello from Raspberry Pi");
	// BEGIN: Special effect. Go to page 100 and press Button 1.
	// Every page becomes P100.
	/* not implemented on VBIT-Pi
	if (BUTTON_GetStatus(BUTTON_1))
	{
		page=0;
	}
    // END: Special effect
	*/
	uint8_t hour, min, sec;
	uint32_t utc;
	uint8_t cbit;
	PacketPrefix((uint8_t*)packet,mag,0);
	packet[5]=HamTab[page%0x10];
	packet[6]=HamTab[page/0x10];
	packet[7]=HamTab[(subcode&0x0f)]; // S1
	subcode>>=4;
	// Map the page settings control bits from MiniTED to actual teletext packet.
	// To find the MiniTED settings look at the tti format document.
	// To find the target bit position these are in reverse order to tx and not hammed.
	// So for each bit in ETSI document, just divide the bit number by 2 to find the target location.
	// Where ETSI says bit 8,6,4,2 this maps to 4,3,2,1 (where the bits are numbered 1 to 8) 
	cbit=0;
	if (control & 0x4000) cbit=0x08;	// C4 Erase page
	packet[8]=HamTab[(subcode&0x07) | cbit]; // S2 add C4
	subcode>>=3;
	packet[9]=HamTab[(subcode&0x0f)]; // S3
	subcode>>=4;
	cbit=0;
	// Not sure if these bits are reversed. C5 and C6 are indistinguishable
	if (control & 0x0002) cbit=0x08;	// C6 Subtitle
	if (control & 0x0001) cbit|=0x04;	// C5 Newsflash
	packet[10]=HamTab[(subcode&0x03) | cbit]; // S4 C6, C5
	cbit=0;
	if (control & 0x0004)  cbit=0x01;	// C7 Suppress Header TODO: Check if these should be reverse order
	if (control & 0x0008) cbit|=0x02;	// C8 Update
	if (control & 0x0010) cbit|=0x04;	// C9 Interrupted sequence
	if (control & 0x0020) cbit|=0x08;	// C10 Inhibit display
	packet[11]=HamTab[cbit]; // C7 to C10
	cbit=(control & 0x0380) >> 6;	// Shift the language bits C12,C13,C14. TODO: Check if C12/C14 need swapping. CHECKED OK.
	if (control & 0x0040) cbit|=0x01;	// C11 serial/parallel
	packet[12]=HamTab[cbit]; // C11 to C14 (C11=0 is parallel, C2,C13,C14 language)
	strncpy(&packet[13],caption,32); // This is dangerously out of order! Need to range check and fill as needed
	// Stuff the page number in. TODO: make it work with hex numbers etc.
	p=strstr(packet,"mpp"); 
	if (p) // if we have mpp, replace it with the actual page number...
	{
		*p++=mag+'0';
		ch=page>>4; // page tens (note wacky way of converting digit to hex)
		*p++=ch+(ch>9?'7':'0');
		ch=page%0x10; // page units
		*p++=ch+(ch>9?'7':'0');
	}
	// Stick the time in. Need to implement flexible date/time formatting
	utc=0;	// This was time in seconds since midnight. This code now runs in buffer.c
	sec=utc%60;
	utc/=60;
	min=utc%60;
	hour=utc/60;
	//if (lastsec!=sec)
	//	xprintf(PSTR("%02d:%02d.%02d "),hour,min,sec);
	//lastsec=sec;
	//sprintf(&packet[37],"%02d:%02d.%02d",hour,min,sec);
	
	// Format the time string in the last 8 characters of the heading
	j=0;
	for (i=37;i<=44;i++)
	{
		if (packet[i]=='\r')	// Replace spurious double height. This can really destroy a TV!
			packet[i]='?';
		if ((packet[i]>='0') && (packet[i]<='9'))
		{
			packet[i]='0';
			switch (j++)
			{
			case 0:packet[i]+=hour/10;break;
			case 1:packet[i]+=hour%10;break;
			case 2:packet[i]+= min/10;break;
			case 3:packet[i]+= min%10;break;
			case 4:packet[i]+= sec/10;break;
			case 5:packet[i]+= sec%10;break;
			default:
				packet[i]='?';
			}
		}
	}
	Parity(packet,13);		
} // Header

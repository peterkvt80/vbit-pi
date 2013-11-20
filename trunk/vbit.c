/** ***************************************************************************
 * Name				: vbit.c
 * Description       : VBIT-Pi Teletext Inserter. Main loop.
 *
 * Copyright (C) 2010-2013, Peter Kwan
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
 *************************************************************************** **/
 /*
  * Currently this is set up to run in root mode.
  */

#include "vbit.h"

void DieWithError(char *errorMessage);  /* Error handling function */

void DieWithError(char *errorMessage)
{
    perror(errorMessage);
    exit(1);
}

void HandleTCPClient(int clntSocket);   /* TCP client handling function */
#define MAXPENDING 5    /* Maximum outstanding connection requests */

/* hic sunt globals */

/* These are set by everyFieldInterrupt
 * and cleared by [name of function that fills the FIFO]
 */
volatile uint8_t gVBIStarted;
struct timespec gFieldTimespec;
volatile uint8_t gOddField;

volatile uint8_t vbiDone; // Set when the timer reckons that the vbi is over. Cleared by main.
volatile uint8_t FIFOBusy;	// When set, the FillFIFO process is required to release the FIFO.
volatile uint8_t fifoReadIndex; /// maintains the tx block index 0..MAXFIFOINDEX-1
volatile uint8_t fifoWriteIndex; /// maintains the load index 0..MAXFIFOINDEX-1

/** everyFieldInterrupt is called.... every field.
 * It sets off the vbi timing sequence.
 * This is the start of the field and vbi will be going out
 * so we need to wait about 1024uS while VBIT sends out the vbi. 
*/ 
void everyFieldInterrupt(void)
{
	// Flash the status LED
	static uint8_t i;
	// Tell the FIFO filler that it can start
	/*result=*/clock_gettime(CLOCK_REALTIME,&gFieldTimespec);	// some sort of Linux timestamp
	if (gVBIStarted==HIGH)
	{
		// Error: FIFO filler thread missing or stuck
	}
	gVBIStarted=HIGH;
	gOddField=digitalRead(GPIO_FLD);
	// printf("S=%d %2d/100\n",gFieldTimespec.tv_sec,gFieldTimespec.tv_nsec);
	// Flip the LED
	if (i==HIGH) i=LOW; else i=HIGH;
	digitalWrite (GPIO_LED,  i) ;
	 
}

void testSPIRam(void)
{
	char mydata[11];
	int i;
	puts("starting test\n");
	for (i=0;i<10;i++) mydata[i]=i+'a'; // stuff the buffer with letters
	// set mux to CPU control
	puts("0:");puts(mydata);			// Letters
	digitalWrite (GPIO_MUX, LOW) ; // MUX to CPU (or is it the other way round?
	WriteSerialRam(mydata,10,0);				// Write buffer to FIFO

	// Stuff the buffer with numbers
	for (i=0;i<10;i++) mydata[i]=i+'0';
	// read the buffer
	puts("1:");puts(mydata);			// Numbers
	WriteSerialRam(mydata,10,0);
	puts("2:");puts(mydata);			// Letters (hopefully)
	puts("ended test\n");
}

/* TRY TO RUN THE SPI DIRECTLY! */
/*
int spitest(void)
{
const static uint8_t     spiMode   = 0 ;		// This is where you would tweak clock phase and polarity
const static uint8_t     spiBPW    = 8 ;
const static uint16_t    spiDelay  = 0 ;
const static uint16_t    spiSpeedDelay  = 0 ;
static uint32_t    spiSpeed ;
	int len;
	int ret;
	puts("Starting spitest\n");
	// Allocate SPI
	int fd = 0;
	fd = open("/dev/spidev0.0", O_RDWR);
	if (fd < 0)
	{
	printf("SPI : can't open device");
	return -1;
	}	
	len=10;
	spiSpeed=100000;
  struct spi_ioc_transfer mode ; // part of the kernel! 
  char data[10];
  mode.tx_buf        = (unsigned long)data ;
  mode.rx_buf        = (unsigned long)data ;
  mode.len           = len ;
  mode.delay_usecs   = spiDelay ;
  mode.speed_hz      = spiSpeed ;
  mode.bits_per_word = spiBPW ;
  mode.cs_change	= 1;	
	
	// Set the SPI mode
	ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
	if (ret == -1)
	{
	printf("SPI : can't set spi mode");
	return 1;
	}



  // see spi_ioc_transfer on Google for details

  return ioctl (fd, SPI_IOC_MESSAGE(1), &mode) ;	
	puts("Ending spitest\n");
	return 0;
}

*/

/** Look for a client 
 * 	This will handle commands but doesn't do anything ATM.
 * Think this will need to go in a thread
 */
void runClient(void)
{
	// Network stuff from http://cs.baylor.edu/~donahoo/practical/CSockets/code/TCPEchoServer.c
    int serverSock;                    /* Socket descriptor for server */
    int clientSock;                    /* Socket descriptor for client */	
    struct sockaddr_in echoServAddr; /* Local address */
    struct sockaddr_in echoClntAddr; /* Client address */	
    unsigned short echoServPort;     /* Server port */
    unsigned int clntLen;            /* Length of client address data structure */

	echoServPort = 5570;  /* This is the local port */

	// System initialisations
	/* Construct local address structure */
    memset(&echoServAddr, 0, sizeof(echoServAddr));   /* Zero out structure */
    echoServAddr.sin_family = AF_INET;                /* Internet address family */
    echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
    echoServAddr.sin_port = htons(echoServPort);      /* Local port */

    /* Create socket for incoming connections */
    if ((serverSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        DieWithError("socket() failed\n");	
	

    /* Bind to the local address */
    if (bind(serverSock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0)
        DieWithError("bind() failed");

    /* Mark the socket so it will listen for incoming connections */
    if (listen(serverSock, MAXPENDING) < 0)
        DieWithError("listen() failed");	
	


	/* Set the size of the in-out parameter */
	clntLen = sizeof(echoClntAddr);

	/* Wait for a client to connect */
	if ((clientSock = accept(serverSock, (struct sockaddr *) &echoClntAddr, 
						   &clntLen)) < 0)
		DieWithError("accept() failed");

	/* clientSock is connected to a client! */

	printf("Handling client %s\n", inet_ntoa(echoClntAddr.sin_addr));

	HandleTCPClient(clientSock);
} // runClient


// Before you run this, you must run setup.sh so that WiringPi is correctly set up.
int main (/* TODO: add args */)
{
	
	int i;
	uint8_t j;
	char str[10];
	char key;
	puts("Welcome to VBIT-Pi\n");
	// Thinks: I2C is only used during setup. We could use the two wires for something else and make the interface smaller.
	// Unless you add something like an MRG keypad then you're stuck.
	wiringPiSetup () ;	// This must be run in root mode

	keypad_init(); // TODO: Use the return 
	i2c_init();		// Does all of the video inits. The video out should start
	pinMode (7, OUTPUT) ;	// The LED. TODO: Alias GPIO names FLD/CSN/MUX
	pinMode (GPIO_MUX, OUTPUT) ;	// The LED. TODO: Alias GPIO names FLD/CSN/MUX
	pinMode (GPIO_CSN, OUTPUT) ;	// SPIRAM Chip select
	
	
	wiringPiISR(GPIO_FLD,INT_EDGE_BOTH,everyFieldInterrupt);	// Enable the field interrupt
	spiram_initialise();
	SetSerialRamStatus(SPIRAM_MODE_SEQUENTIAL);	
//	testSPIRam();
	
	// Set up the eight magazine threads
	magInit();

	// TODO: Test that the threads started.
	
	// Sequence streams of packets into VBI fields
	i=piThreadCreate(Stream);
	if (i != 0)
	{
		printf ("Stream thread! It didn't start\n");		
		return 1;
	}
	// Copy fields of VBI into the FIFO
	i=piThreadCreate(FillFIFO);
	if (i != 0)
	{
		printf ("FillFIFO thread! It didn't start\n");
		return 1;
	}
	while (1)
	{
		// printf("Starting the loop\n");
		fflush(stdout);	// Force output so it doesn't buffer it forever
		delay(1);	
		for (i=0;i<8;i++)
		{
			if (i!=5)	// Avoid SAA7113 chip which shares the address
			{
				j=read_keychar(i);
				if (j>0 && j<255)
				{
					key='C';
					printf("Key[%d]=%d\n",i,j);
					switch (j)
					{
					case 48: key='C';break;
					case 49: key='9';break;
					case 50: key='8';break;
					case 51: key='7';break;
					case 52: key='S';break;
					case 53: key='6';break;
					case 54: key='5';break;
					case 55: key='4';break;
					case 56: key='D';break;
					case 57: key='3';break;
					case 58: key='2';break;
					case 59: key='1';break;
					case 60: key='I';break;
					case 61: key='>';break;
					case 62: key='0';break;
					case 63: key='<';break;
					}
					sprintf(str,"DISPLAY=:0 xdotool search --name menu key %c",key);
					puts(str);
					system(str);
				}
			}
		}
		// runClient();
		
	}
	puts("Finished\n"); // impossible to get here
	return 1;
}


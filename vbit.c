/** Copyright notice goes here
 * 
 */
 /*
  * Currently this is set up to run in root mode.
  */

#include "vbit.h"

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
	const uint32_t day=(uint32_t)60*60*24;
	// Flash the status LED
	static uint8_t i;
	static uint8_t count;
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

int main (/* TODO: add args */)
{
	
	char mydata[10];
	int i;
	puts("Welcome to VBIT-Pi\n");
	// System initialisations
	// Thinks: I2C is only used during setup. We could use the two wires for something else and make the interface smaller.
	wiringPiSetup () ;	// This must be run in root mode
	
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
		fflush(stdout);	// Force output so it doesn't buffer ir forever
		delay (1000) ;		
	}
	puts("Finished\n"); // impossible to get here
	return 1;
}


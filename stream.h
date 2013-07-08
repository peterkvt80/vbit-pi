#ifndef _STREAM_H_
#define _STREAM_H_

//#include <fcntl.h>
//#include <sys/ioctl.h>
//#include <linux/spi/spidev.h>
//#include <time.h>

#include <stdio.h>
//#include <stdlib.h>
//#include <unistd.h>
#include <stdint.h>

//#include <stdint.h>
// For the timing
#include <wiringPi.h>

// VBIT
#include "buffer.h"
#include "mag.h"

//#include "fifo.h"
//#include "pins.h"
//#include "FILLFifo.h"

/** Stream is a thread that 
1) sinks packets from mag.c
2) Handles packet priority and sequencing
3) inserts special packets 8/30, databroadcast etc.
4) sources packets to FillFIFO
*/
PI_THREAD (Stream);
#define STREAMBUFFERSIZE 50
extern bufferpacket streamBuffer[STREAMBUFFERSIZE];

#endif

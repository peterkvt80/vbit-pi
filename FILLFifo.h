#ifndef _FILLFIFO_H_
#define _FILLFIFO_H_

// Standard includes
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <stdint.h>

// I/O library
#include <wiringPi.h>

// VBIT
#include "i2c.h"
#include "fifo.h"
#include "pins.h"
#include "packet.h"
#include "stream.h"

int FILLFifo(void);
struct timespec diff(struct timespec start, struct timespec end);

extern volatile uint8_t gVBIStarted;
extern struct timespec gFieldTimespec;
extern volatile uint8_t gOddField;
PI_THREAD(FillFIFO);
#endif

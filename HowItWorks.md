# Introduction #

VBIT-Pi is written in C and is designed to run on the Raspberry Pi with VBIT-pi hardware. It makes use of the WiringPi library. The teletext pages are in MRG tti format and are played directly from the SD card.

The software is arranged as a set of threads. Eight magazine threads generate packet streams. These are multiplexed by another thread into transmission order and output to another thread that jumps in to fill the fifo immediately after VBI.

# Details #
setup.sh calls the WiringPi gpio facility which configures the gpio, spi and I2C. This must be run before the vbit code.

The VBIT-pi code itself is split into many C files.
## mag.c ##
mag.c converts teletext files into streams of packets. It first scans all of the ~/Pages folder and then streams out a magazine to stream.c. There is one thread per magazine. mag.c looks after the carousels. Carousels are handled by a special list that maintains the state of each carousel.
## packet.c ##
packet.c has routines for assembling packets. From a raw packet it must create an ham encoded MRAG, header packets and control bits. It then reverses bits into transmission order and performs odd parity. It is mainly called by mag.c to help in creating transmission ready packets.
## stream.c ##
Stream.c multiplexes packets from all eight magazine threads and combines them into one transmission stream. It handles magazine priority and assigns a number from 1 to 10 where 10 is the slowest. As delivered, mags 1 and 2 are highest priority and the others have diminishing priority.
## buffer.c ##
Buffer.c is used to transfer packets between threads. It has one major role where it comes to headers. stream.c calls a routine that moves packets from one mag thread to the output stream. This routine decodes and detects headers and when headers are found it replaces the incoming header for one with a proper station ident and the correct time.
## FILLFifo.c ##
This runs a thread which takes multiplexed stream of packets and stuffs them into the FIFO. It spends much of it's time waiting for the VBI to be complete before it is allowed to write the next field into the FIFO. It sees a flag that gets set by interrupt. It clears this flag and calculates how much longer it has to wait. Then it fills the FIFO with the next field and calculates how long it will be until it needs to check for the next interrupt. It then sets the FIFO to be controlled by the teletext encoder and waits.
## vbit.c ##
vbit.c is the main code. It only sets up the various threads and then loops doing nothing much.
## tables.c ##
This contains lookup tables for ham, deham, parity and nybble reversing.
## page.c ##
Parses MRG Systems teletext pages with tti file extension.
## i2c.c ##
Routines to initialise the video decoder and encoder.
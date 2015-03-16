# VBIT-Pi - Teletext inserter add-on for Raspberry Pi #
VBIT-Pi is a teletext inserter add-on for the Raspberry Pi.

More about VBIT-Pi on http://teastop.co.uk/vbit-pi/

Features:
  * Delivers a full teletext service.
  * All pages are streamed off the Pi's SD card.
  * The video path is via a digital encoder and decoder and needs no adjustments.
  * The teletext is generated directly by a Philips chip. It is specifically designed to       produce an accurate text signal with raised cosine filtering. Most homebrew designs just have a digital switch directly on the video which creates a big bunch of unwanted harmonics.
  * The encoder can also support closed captions but you'd have to study the datasheets and implement it yourself.
  * Uses parallel transmission so the minimum of lines are wasted.
  * A traditional FIFO ensures that the teletext stream goes out without losing packets.
  * Code is entirely C and runs on the Pi so the development cycle is simply edit/make/run
  * Linux fetches the time by network and this is used to generate the header clock and the packet 8/30 format 1 time used to set VCRs and certain TV sets.
  * Spare I/O is brought out to a pin array for adding LEDs, switches or serial port.

It can generate a full text service from pages on the SD card. It can also generate time signals so that certain TVs and VCRs can set their time.

The VBIT-Pi board has the same dimensions size as the Raspberry Pi. It is connected by a short 26 way cable to the Raspberry Pi GPIO port. VBIT-Pi requires composite input video to insert on. Video in and out is via BNCs. There is also an LED which will be used to indicate a good video signal. All the spare GPIO has been brought out to a pin array so you can add more peripherals.

All the code is written in C and makes extensive use of the WiringPi library. A VBIT setup will consist of an ethernet enabled Raspberry Pi, a VBIT-Pi board, PAL video signal input. Then install WiringPi version 2. See Gordon's instructions here https://projects.drogon.net/raspberry-pi/wiringpi/download-and-install/

Put the vbit-pi source code somewhere handy like /home/pi/vbit-pi
The code is available on svn.

Install subversion: sudo apt-get install subversion

Then go to the Source page and checkout the project with Subversion.

Put the sample tti pages code in /home/pi/Pages/

Connect Raspberry Pi and vbit-pi with the 26 way connector.

Attach a stable PAL video source to the vbit-pi input.

Attech a teletext TV to the vbit-pi

Apply power.

In a shell console go to the teletext folder

$ cd vbit-pi

Set up the hardware (the first line you only need to do the first time)

$ sudo chmod 777 setup.sh

$ ./setup.sh

Build the executable

$ make

Run teletext

$ sudo ./vbit-pi


# Latest News #

20-Oct-2013

Added code to support an MRG Systems I2C keypad. It isn't really useful but somebody might have different ideas.

Also working on some PHP that will scrape BBC News and make a Ceefax service out of it. If you prefer browsing the news with a TV remote control then this will be the perfect download for you.

13-Aug-2013

Carousels finally work properly. The system is currently configured for up to 16 carousels per magazine. To make a carousel of pages, just copy all of the pages into a one file. Change the page number to PN,xxx01 and the subcode to SC,0001. The next page is PN,xxx02 and SC,0002. The first CT number gives the timing for the whole set. CT,15,T would cycle with a 15 second time.

01-Aug-2013

There are a few built and tested boards now. Carousels work well, but there is a minor bug that pops up occasionally. Packet 8/30 is not a priority at the moment. TCP/IP port listener for controlling VBIT-Pi is implemented but doesn't do anything yet except echo data.

08-Jul-2013

Boards are in production. Software is producing a basic service and is on SVN. This week I'll implement carousels, packet 8/30 format 1 time. Header row 0 code seems to be bug free at last.

## Future Developments ##
In future there will be special interfaces for doing rapid updates for unusual things like animation. This couldn't be done on a broadcast transmission because it would use up all of the bandwidth.

At the moment there are no control interfaces but I'd hope that telnet and a web interface will be coming along.

All the settings are in source code at the moment. I'll work out how GLIB works and use the configuration features of that.
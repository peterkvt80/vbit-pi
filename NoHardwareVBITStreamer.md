# Introduction #

By fiddling with the video registers it is possible to get Raspberry Pi to put video in the vertical blanking interval. By careful timing, it is also possible to synchronise to the video sync. So by painting the correct patterns into the video buffers at the right time it is possible to create a teletext service.

To create a service you need pages, a page packetiser, and a packet to video output. I have written a script to get some old ITV pages, the VBIT packetiser and the raspi-teletext packet to video converter.

# Details #

If you want to see what is being installed then please examine the getteletext script. Any questions just email me peterk.vt80@gmail.com

Short Instructions
  * Create a Rasbian boot SD card for the Pi. Google Rasbian Download for details.
  * Boot your Pi with the card and set the Locale and Timezone. You can also expand your file partition at this time.
  * sudo nano /boot/config.txt and uncomment the line for normal PAL:  sdtv\_mode=2    then reboot.
  * Now type these commands. It will load a configuration script then run it.

> wget -O getteletext http://teastop.co.uk/teletext/vbit/getteletext

> chmod +x getteletext

> ./getteletext

Once you have installed it you can just type this command that has been installed in the home folder /home/pi.

./go

Finally connect the video to a TV set and press the TEXT button.

Notes:
If you get a lot of compile errors in raspi-teletext you should download a current Rasbian and install that first. raspi-teletext will not compile on older versions of Rasbian.
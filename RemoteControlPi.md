# Introduction #

The Raspberry Pi can be controlled from another PC. For Windows users please install WinSCP. You need your Pi and your PC to be on the same network.

You can copy files to and from your pi using the graphical interface. This is most useful for uploading pages for transmission.


# Details #

First you need to know the IP address of the Raspberry Pi. Connect a monitor and keyboard to the Pi. Login and type ifconfig. You should get an IP address and by default this will be a dynamic address supplied by your network, such as 192.168.0.65. It would be a good idea to set a static IP address so that it won't change each time that you reboot. Google for how to set a static ip.

# WinSCP #
WinSCP is used to copy files. When you configure WinSCP, start a new session giving the IP address. You will then be able to log into the Pi using pi/raspberry as the username and password.
To copy teletext pages, move them into the folder /home/pi/Pages. When the vbit-pi is restarted then the Pages will be transmitted.

# PuTTY #
Putty is a terminal program. It gives you a remote system console. Any commands that you could type directly into the Pi can also be done on your remote PC. PuTTY should have been installed as a side-effect of installing WinSCP. If so, then you can start PuTTY while you are in WinSCP by typing CTRL-P.
# Introduction #

There are no configuration files or interfaces with the vbit-pi software. This is because I haven't written any. So what you have to do is hack the software to change it for what you want to do. The first thing that you would want to do with a vbit-pi is change the service header so I will describe how to do that for complete beginners.

For these instructions you need a TV and a USB keyboard plugged into the Pi and an SD card that boots and runs a teletext service.


# Logging in and stopping teletext #

Connect the TV to the yellow monitor phono on the Pi. You will see some debug information scroll up indicating that the teletext service is alive. Press Enter. The system should respond with:
> Raspberry login: pi
> Password: raspberry

It will be annoying because the teletext program is running in the background and often will put up messages. Just ignore these.

If you logged in OK then when you press Enter the system will print the prompt:
> pi@raspberrypi $
To stop the teletext and make the messages stop, type in
> sudo killall -9 vbit

# Editing the source code #
You will find all the code in the vbit-pi directory. Change directory to there and then see what is in there:
> cd vbit-pi

> ls
The easiest editor is nano and in this case the header is in buffer.c
> nano buffer.c
Using the cursor keys scroll down until you find a line that has
> char template("MPP PiFAX  1 DAY dd MTH hh:mm/ss");
Be super careful with this because it must have exactly 32 characters in it. When you are done editing, press CTRL-O to save and CTRL-X to exit.
# Making the executable #
To compile the code type
> make
There may be warnings but there must be no errors. If there are errors then correct the source code and try another make.
# Restart vbit #
You can now just cycle the power and wait for the system to start. Or to start the system quicker type:
> sudo ./vbit
Your new header should be active.
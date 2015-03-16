# Introduction #

Time and locale affect the service header. This is the top row of a teletext page. The time is used to display the date and time. The locale determines the language that the system uses. In theory vbit-pi should use the locale to set the day and month but for some reason it doesn't work.

How does time work on vbit-pi? It needs an internet connection. The Pi fetches the time from an NTP server. The Pi then maintains this time as long as it is running. The current time is injected into the headers as it runs. The time is usually configured to London as that is my nearest city.

Log into your Pi and type
> sudo raspi-config

# Timezone #
Scroll down to the change\_timezone option. Select your continent and your nearest city. Mine is Europe/London.
# Locale #
Locales are used to choose a language for the Linux system. Please use the UTF-8 version for your country. For example, English, Great Britain, UTF-8 is called:
> [**] en\_GB.UTF-8 UTF-8
Select a language by scrolling down to it and pressing space. The [**] indicates that it is selected.
Germany is probably
> de\_DE.UTF-8 UTF-8
etc.
Then select OK and you will be able to choose the default locale. This is where you can set your own.
Note that the time and date names in the vbit-pi header are not affected by locale. You must change the names in a different way [I can't remember how to make it work](that.md)
#Code to import teletext40 raw pages

# How to convert teletext40 raw pages to ttix teletext on a Raspberry Pi #

1)
Get the teletext pages.
```
wget -O frames.csv http://www.teletext40.com/data/frames.csv
```
2)

Below is the code that you can run on a Pi. The source code for ttx40.c and the Makefile. Put these two files onto the Pi and type make. It should compile OK.

3)

Type
```
cat frames.csv > ./ttx40
```
This will create a lot of ttx4tnnn.ttix pages.

4)

Copy these pages to the Pages folder. (you probably want to delete the existing pages in Pages folder first
```
cp *.ttix /home/pi/Pages
```



# Code #

Create files ttx40.c and Makefile and copy the following code into them.


ttx40.c
```
/** ttx40 - Teletext page converter
 * Converts Teletext40 raw data to MRG Systems tti 
 * tti is used by MiniTED, wxTED, Droidfax, VBIT.
 * Typical usage: 
 * cat frames.csv | ./ttx40
 * 
 * Just put the resulting pages into the Pages folder of VBIT.
 * For Droidfax you might need to change the filename prefix in the Processing 2 code.
 *
 * Copyright (c) 2015 Peter Kwan
 * Insert [The MIT License (MIT)] here.
 */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#define MAXCHAR 3000
int main (int argc, char **argv)
{
	char txt[MAXCHAR];
	char *p;
	char *page=NULL;
	char *subpage=NULL;
	char *name=NULL;
	char hex[3];
	uint16_t lastPage=999;
	uint16_t thisPage=999;
	uint16_t sub=999;
	uint8_t line=99;
	uint8_t i;
	uint32_t ch;
	hex[2]=0;
	FILE *fs=NULL;
	char fn[50];
	while (!feof(stdin))
	{
		// Read a page
		fgets(txt, MAXCHAR, stdin);
		// Split the csv fields
		page=strtok(txt,",");
		if (!page) break;
		subpage=strtok(NULL,",");
		if (!subpage) break;
		name=strtok(NULL,",");
		if (!name) break;
		p=strtok(NULL,",");
		// Decode the numbers
		thisPage=atoi(page);
		sub=atoi(subpage);
		if (thisPage!=lastPage)
		{
			// Open a new file
			if (fs) fclose(fs);
			sprintf(fn,"ttx4t%d.ttix",thisPage);
			fs=fopen(fn,"w");
			lastPage=thisPage;
		}
		fprintf(fs,"DE, Teletext40 page. Author=%s\n",name);
		fprintf(fs,"PN,%s%02d\n",page,sub);
		fprintf(fs,"CT,7,T\n"); // Might not be a great idea
		fprintf(fs,"SC,%04d\n",sub);
		fprintf(fs,"PS,8000\n");
		fprintf(fs,"MS,0\n");
		for (line=1;line<25;line++)
		{
			fprintf(fs,"OL,%d,",line);	// New line
			for (i=0;i<40;i++)
			{
				hex[0]=*p++;									// Get the high nybble
				hex[1]=*p++;									// Get the low nybble
				ch=(char)strtol(hex,NULL,0x10) & 0x7f;			// Convert to hex and remove bit 7
				if (ch<0x20)
				{
					fputc(0x1b,fs);
					ch+=0x40;								// viewdata escape (for Droidfax)
					// i=0;
				}
				fputc(ch,fs);
			}
			fputc('\n',fs);
		}
	}
	fputc('\n',fs);
	fclose(fs);
	return 0;
}
```

Makefile
```
CFLAGS+=-DSTANDALONE -D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS -DTARGET_POSIX -D_LINUX -fPIC -DPIC -D_REENTRANT -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -U_FORTIFY_SOURCE -Wall -g -DHAVE_LIBOPENMAX=2 -DOMX -DOMX_SKIP64BIT -ftree-vectorize -pipe -DUSE_EXTERNAL_OMX -DHAVE_LIBBCM_HOST -DUSE_EXTERNAL_LIBBCM_HOST -DUSE_VCHIQ_ARM -Wno-psabi

LDFLAGS+=-L$(SDKSTAGE)/opt/vc/lib/ -lGLESv2 -lEGL -lopenmaxil -lbcm_host -lvcos -lvchiq_arm -lpthread -lrt -lm -L../libs/ilclient -L../libs/vgfont

INCLUDES+=-I$(SDKSTAGE)/opt/vc/include/ -I$(SDKSTAGE)/opt/vc/include/interface/vcos/pthreads -I$(SDKSTAGE)/opt/vc/include/interface/vmcs_host/linux -I./ -I../libs/ilclient -I../libs/vgfont

OFILES=ttx40.o

all: ttx40

ttx40: $(OFILES)
	$(CC) -o $@ -Wl,--whole-archive $(OFILES) $(LDFLAGS) -Wl,--no-whole-archive -rdynamic

%.o: %.c
	@rm -f $@ 
	$(CC) $(CFLAGS) $(INCLUDES) -g -c $< -o $@ -Wno-deprecated-declarations

clean:
	rm -f *.o ttx40
```
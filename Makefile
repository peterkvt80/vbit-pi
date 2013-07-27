#Change output_file_name to your desired exe filename

#Set the compiler you are using ("gcc" for C or "g++" for C++)
CC=gcc

#Set any compiler flags you want to use (e.g. "-I."), or leave blank
CFLAGS = -g -O2 -Wall -I.

#Set any dependant files (e.g. header files) so that if they are edited they cause a re-compile (e.g. "main.h my_sub_functions.h some_definitions_file.h"), or leave blank
DEPS = pins.h

#Set all your object files (the object files of the .c files in your project, e.g. "main.o my_sub_functions.o")
OBJ = vbit.o				\
	i2c.o \
	fifo.o \
	packet.o \
	tables.o \
	stream.o \
	mag.o \
	buffer.o \
	page.o \
	FILLFifo.o \
	HandleTCPClient.o

#Any special libraries you are using (e.g. "-lbcm2835"), or leave blank
LIBS = -lwiringPi -lrt

#Below here doesn't need to change
#Compile each object file
%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

#Combine them into the output file
vbit: $(OBJ)
	gcc -o $@ $^ $(CFLAGS) $(LIBS)

#Cleanup
.PHONY: clean

clean:
	rm -f *.o *~ core *~ 

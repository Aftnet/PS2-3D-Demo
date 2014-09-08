TARGET = main

CC = g++

.SUFFIXES: .cpp .o .vcl

OBJS = 	main.o \
		ps2maths.o \
		ps2matrix4x4.o \
		ps2vector4.o \
		ps2quaternion.o \
		pad.o \
		dma.o \
		sps2wrap.o \
		texture.o \
		font.o \
		vu1code.o \
		pipeline.o \
		timer.o \
		vumanager.o \
		ms3dmodel.o

LIBS = -lm -lsps2util

CFLAGS = -g

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LIBS)

.cpp.o:
	$(CC) -c $< $(CFLAGS) -o $@
	
.vcl.o:
	vcl -g -o$*.vsm $*.vcl
	ee-dvp-as -o $*.vo_ $*.vsm
	objcopy -Obinary $*.vo_ $*.bin_
	./bin2as $* $*.bin_ > $*.a_
	as -mcpu=r5900 -KPIC -mips2 -o $*.o $*.a_
	rm $*.vo_ $*.bin_ $*.a_

*.vcl: bin2as
bin2as:  bin2as.cpp
	cc -o bin2as bin2as.cpp

clean:
	rm -f $(TARGET)
	rm -f $(OBJS)
	rm -f .depend


# create the dependancy file	
depend:
	$(CC) $(CFLAGS) -MM *.cpp > .depend


#include the dependencies into the build
-include .depend
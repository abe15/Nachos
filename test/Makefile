# use normal make for this Makefile
#
# Makefile for building user programs to run on top of Nachos
#
# Several things to be aware of:
#
#    Nachos assumes that the location of the program startup routine (the
# 	location the kernel jumps to when the program initially starts up)
#       is at location 0.  This means: start.o must be the first .o passed 
# 	to ld, in order for theroutine "Start" to be loaded at location 0
#

# If you are running on SunOS 5.x machines, uncomment the next line
GCCDIR = /auto/home-scf-22/csci402/testgnu/decstation-ultrix/bin/
LDFLAGS = -T newscript -N
ASFLAGS = -mips1
CPPFLAGS = $(INCDIR)

# if you aren't cross-compiling:
# GCCDIR =
# LDFLAGS = -N -T 0
# ASFLAGS =
# CPPFLAGS = -P $(INCDIR)


CC = $(GCCDIR)gcc
AS = $(GCCDIR)as
LD = $(GCCDIR)ld
CPP = $(GCCDIR)cpp

INCDIR =-I../userprog -I../threads
CFLAGS = -G 0 -c $(INCDIR)

all: halt matmult sort testfiles multiprogramming grpregtest grpregfunc totalordtest totalordfunc manager cook waiter cust ordtaker carljunior
 


start.o: start.s ../userprog/syscall.h
	$(CPP) $(CPPFLAGS) start.s > strt.s
	$(AS) $(ASFLAGS) -o start.o strt.s
	rm strt.s




halt.o: halt.c
	$(CC) $(CFLAGS) -c halt.c
halt: halt.o start.o
	$(LD) $(LDFLAGS) start.o halt.o -o halt.coff
	../bin/coff2noff halt.coff halt

waiter.o: waiter.c
	$(CC) $(CFLAGS) -c waiter.c
waiter: waiter.o start.o
	$(LD) $(LDFLAGS) start.o waiter.o -o waiter.coff
	../bin/coff2noff waiter.coff waiter


carljunior.o: carljunior.c
	$(CC) $(CFLAGS) -c carljunior.c
carljunior: carljunior.o start.o
	$(LD) $(LDFLAGS) start.o carljunior.o -o carljunior.coff
	../bin/coff2noff carljunior.coff carljunior


ordtaker.o: ordtaker.c
	$(CC) $(CFLAGS) -c ordtaker.c
ordtaker: ordtaker.o start.o
	$(LD) $(LDFLAGS) start.o ordtaker.o -o ordtaker.coff
	../bin/coff2noff ordtaker.coff ordtaker

sort.o: sort.c
	$(CC) $(CFLAGS) -c sort.c
sort: sort.o start.o
	$(LD) $(LDFLAGS) start.o sort.o -o sort.coff
	../bin/coff2noff sort.coff sort

cust.o: cust.c
	$(CC) $(CFLAGS) -c cust.c
cust: cust.o start.o
	$(LD) $(LDFLAGS) start.o cust.o -o cust.coff
	../bin/coff2noff cust.coff cust

matmult.o: matmult.c
	$(CC) $(CFLAGS) -c matmult.c
matmult: matmult.o start.o
	$(LD) $(LDFLAGS) start.o matmult.o -o matmult.coff
	../bin/coff2noff matmult.coff matmult


grpregtest.o: grpregtest.c
	$(CC) $(CFLAGS) -c grpregtest.c
grpregtest: grpregtest.o start.o
	$(LD) $(LDFLAGS) start.o grpregtest.o -o grpregtest.coff
	../bin/coff2noff grpregtest.coff grpregtest


manager.o: manager.c
	$(CC) $(CFLAGS) -c manager.c
manager: manager.o start.o
	$(LD) $(LDFLAGS) start.o manager.o -o manager.coff
	../bin/coff2noff manager.coff manager



cook.o: cook.c
	$(CC) $(CFLAGS) -c cook.c
cook: cook.o start.o
	$(LD) $(LDFLAGS) start.o cook.o -o cook.coff
	../bin/coff2noff cook.coff cook


grpregfunc.o: grpregfunc.c
	$(CC) $(CFLAGS) -c grpregfunc.c
grpregfunc: grpregfunc.o start.o
	$(LD) $(LDFLAGS) start.o grpregfunc.o -o grpregfunc.coff
	../bin/coff2noff grpregfunc.coff grpregfunc

totalordfunc.o: totalordfunc.c
	$(CC) $(CFLAGS) -c totalordfunc.c
totalordfunc: totalordfunc.o start.o
	$(LD) $(LDFLAGS) start.o totalordfunc.o -o totalordfunc.coff
	../bin/coff2noff totalordfunc.coff totalordfunc




totalordtest.o: totalordtest.c
	$(CC) $(CFLAGS) -c totalordtest.c
totalordtest: totalordtest.o start.o
	$(LD) $(LDFLAGS) start.o totalordtest.o -o totalordtest.coff
	../bin/coff2noff totalordtest.coff totalordtest

testfiles.o: testfiles.c
	$(CC) $(CFLAGS) -c testfiles.c
testfiles: testfiles.o start.o
	$(LD) $(LDFLAGS) start.o testfiles.o -o testfiles.coff
	../bin/coff2noff testfiles.coff testfiles



multiprogramming.o: multiprogramming.c
	$(CC) $(CFLAGS) -c multiprogramming.c
multiprogramming: multiprogramming.o start.o
	$(LD) $(LDFLAGS) start.o multiprogramming.o -o multiprogramming.coff
	../bin/coff2noff multiprogramming.coff multiprogramming


clean:
	rm -f *.o *.coff

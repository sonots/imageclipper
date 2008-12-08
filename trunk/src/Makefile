# Makefile
# Change ~/usr/ to your install location, e.g., /usr/
# Change boost-1_36 to your version
# Change -gcc41-mt to yours. ls ~/usr/lib
# $ make check
# to check you have boost libraries

CC = g++
LINK = g++
INSTALL = install
CFLAGS = `pkg-config --cflags opencv` -I ~/usr/include/boost-1_36 -I.
LFLAGS = `pkg-config --libs opencv` -L ~/usr/lib -lboost_system-gcc41-mt -lboost_filesystem-gcc41-mt
all: imageclipper

imageclipper.o: imageclipper.cpp
	$(CC) $(CFLAGS) -o $@ -c $^

imageclipper: imageclipper.o
	$(LINK) -o $@ $^ $(LFLAGS)

check:
	ls -d ~/usr/include/boost-1_36
	ls ~/usr/lib/libboost_system-gcc41-mt.a
	ls ~/usr/lib/libboost_filesystem-gcc41-mt.a

clean:
	rm -f imageclipper *.o

install:
	cp imageclipper ~/usr/bin/

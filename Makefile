CC = gcc
LD = gcc
CFLAGS = -O0 -Wall -g

.PHONY: all nc nc_compile nc_link ncs ncs_compile ncs_link clean

all: | clean nc	ncs

nc:	nc_compile nc_link

nc_compile: src/nc/nc.c src/common/simplesocket.c src/common/simplesocket.h
	$(CC) $(CFLAGS) -c src/nc/nc.c src/common/simplesocket.c

nc_link: nc.o simplesocket.o 
	$(LD) -o nc nc.o simplesocket.o

ncs: ncs_compile ncs_link

ncs_compile: src/ncs/ncs.c src/common/simplesocket.c src/common/simplesocket.h
	$(CC) $(CFLAGS) -c src/ncs/ncs.c src/common/simplesocket.c

ncs_link: ncs.o simplesocket.o 
	$(LD) -o ncs ncs.o simplesocket.o

clean:
	rm -f *.o nc ncs


CC = gcc
LD = gcc
CFLAGS = -O0 -Wall -g

.PHONY: all nc nc_compile nc_link ncs ncs_compile ncs_link ws ws_compile ws_link clean

all: | clean nc	ncs ws

nc:	nc_compile nc_link

nc_compile: src/nc/nc.c src/common/simplesocket.c src/common/simplesocket.h
	$(CC) $(CFLAGS) -c src/nc/nc.c src/common/simplesocket.c

nc_link: nc.o simplesocket.o 
	$(LD) -o nc nc.o simplesocket.o

ncs: ncs_compile ncs_link

ncs_compile: src/ncs/ncs.c src/common/simplesocket.c src/common/simplesocket.h src/common/fds.h src/common/fds.c
	$(CC) $(CFLAGS) -c src/ncs/ncs.c src/common/simplesocket.c src/common/fds.c

ncs_link: ncs.o simplesocket.o fds.o
	$(LD) -o ncs ncs.o simplesocket.o fds.o

ws: ws_compile ws_link

ws_compile: src/ws/ws.c src/common/simplesocket.c src/common/simplesocket.h src/common/fds.h src/common/fds.c
	$(CC) $(CFLAGS) -c src/ws/ws.c src/common/simplesocket.c src/common/fds.c src/common/buffer.c

ws_link: simplesocket.o fds.o buffer.o ws.o
	$(LD) -o ws ws.o simplesocket.o fds.o buffer.o

clean:
	rm -f *.o nc ncs ws


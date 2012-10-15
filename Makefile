CC = gcc
LD = gcc
CC_OPTS = -O0 -Wall -g

all: nc

nc: nc_compile nc_link

nc_compile:
	$(CC) $(CC_OPTS) -c src/nc/nc.c src/common/simplesocket.c

nc_link: nc_compile
	$(LD) -o nc nc.o simplesocket.o

clean:
	rm -f *.o nc


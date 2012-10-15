CC = gcc
LD = gcc
CC_OPTS = -O0 -Wall -g

all: compile link

compile:
	$(CC) $(CC_OPTS) -c src/*.c

link:
	$(LD) -o nc *.o

clean:
	rm -f *.o nc


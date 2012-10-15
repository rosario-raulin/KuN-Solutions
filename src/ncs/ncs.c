#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "../common/simplesocket.h"
#include "../common/fds.h"

#define DEFAULT_PORT "8080"
#define DEFAULT_BUFSIZE 8192
#define CLIENT_HUP -1

static int verbose = 0;

static int
echo_client(int client) {
	int bytes_read;
	char buf[DEFAULT_BUFSIZE];
	if ((bytes_read = read(client, buf, DEFAULT_BUFSIZE - 1)) <= 0) {
		return CLIENT_HUP;
	} else {
		buf[bytes_read] = '\0';
		write(client, buf, bytes_read + 1);
		if (verbose) { printf(">> %s", buf); }
		return 0;
	}
}

static fds* 
handle_input(fds* f, int fd, int pos) {
	if (fd == f->p[0].fd) {
		int next;
		if ((next = accept(f->p[0].fd, NULL, NULL)) != -1) {
			return fds_add(f, next, POLLIN);
		}
	} else {
		if (echo_client(fd) == CLIENT_HUP) {
			return fds_remove(f, pos);
		}
	}
	return f;
}

static void
handle_clients(int s) {

	fds* f = fds_create();
	f = fds_add(f, s, POLLIN);
	if (f == NULL) {
		fprintf(stderr, "error: insufficient memory!\n");
	}
	
	int nrevs;
	while ((nrevs = poll(f->p, f->size, -1)) > 0) {
		int i;
		int handled;

		for (i = handled = 0; i < f->size && handled < nrevs; ++i) {
			switch (f->p[i].revents) {
				case POLLIN:
					++handled;
					if (handle_input(f, f->p[i].fd, i) == NULL) {
						fprintf(stderr, "error: insufficient memory!\n");
						exit(EXIT_FAILURE);
					}
					break;

				case 0:
					break;
				
				default:
					fprintf(stderr, "warning: revents: %#x\nfrom: %d!\n", 
						f->p[i].revents, i);
					break;
			}
		}
	}
}

int
main(int argc, char* argv[]) {

	char* port = DEFAULT_PORT;
	int opt;
	while ((opt = getopt(argc, argv, "p:v")) != -1) {
		switch (opt) {
			case 'p':
				port = optarg;
				break;
			case 'v':
				verbose = 1;
				break;
		}
	}

	int s = ssock_v4(port);
	if (s == -1) {
		fprintf(stderr, "error: %s\n", SOCK_ERR);
		return EXIT_FAILURE;
	}

	handle_clients(s);
	return EXIT_SUCCESS;
}


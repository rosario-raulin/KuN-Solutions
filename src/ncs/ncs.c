#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "../common/simplesocket.h"

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
		write(client, buf, bytes_read + sizeof('\0'));
		if (verbose) { printf(">> %s", buf); }
		return 0;
	}
}

static void
handle_clients(int s) {

	int capacity = DEFAULT_BUFSIZE;
	int nfds = 1;
	struct pollfd* fds = calloc(capacity, sizeof(struct pollfd));
	if (!fds) {
		fprintf(stderr, "error: insufficient memory!\n");
		exit(EXIT_FAILURE);
	}

	fds[0].fd = s;
	fds[0].events = POLLIN;

	int nrevs;
	while ((nrevs = poll(fds, nfds, -1)) > 0) {
		int i;
		int handled;

		for (i = handled = 0; i < nfds && handled < nrevs; ++i) {
			switch (fds[i].revents) {
				case POLLIN:
					++handled;
					if (fds[i].fd == s) {
						if (nfds == capacity) {
							capacity *= 2;
							if ((fds = realloc(fds, capacity)) == NULL) {
								fprintf(stderr, "error: insufficient memory!\n");
								exit(EXIT_FAILURE);
							}
						}
						if ((fds[nfds].fd = accept(s, NULL, NULL)) == -1) {
							fprintf(stderr, "error: accept() failed!\n");
							continue;
						}
						fds[nfds++].events = POLLIN;
					} else {
						if (echo_client(fds[i].fd) == CLIENT_HUP) {
							close(fds[i].fd);
							fds[i] = fds[--nfds];
						}
					}
					
					break;
				case 0:
					break;

				default:
					fprintf(stderr, "warning: revents: %#x\nfrom: %d!\n",
									fds[i].revents, i);
					break;
			}
		}
	}

	free(fds);
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


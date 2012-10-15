#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <poll.h>

#include "simplesocket.h"

#define BUFSIZE 8192

static void
io(char* host, char* port) {
	int s = csock_v4(host, port);

	if (s < 0) {
		fprintf(stderr, "error: %s!\n", SOCK_ERR);
	} else {
		struct pollfd fds[2];
		fds[0].fd = STDIN_FILENO;
		fds[1].fd = s;
		fds[0].events = fds[1].events = POLLIN;

		int bytes_read;
		char buf[BUFSIZE];
		while (poll(fds, 2, -1) > 0) {
			if (fds[0].revents == POLLIN) {
				if ((bytes_read = read(STDIN_FILENO, buf, BUFSIZE)) > 0) {
					write(s, buf, bytes_read);
				}
			}
			if (fds[1].revents == POLLIN) {
				if ((bytes_read = read(s, buf, BUFSIZE-1)) > 0) {
					buf[bytes_read] = '\0';
					printf("%s", buf);
				}
			}
		}
	}
}

static void
print_usage(char* appname) {
	fprintf(stderr, "usage: %s -s server -p port\n", appname);
}

int
main(int argc, char* argv[]) {

	char* server = NULL;
	char* port = NULL;

	int opt;
	while ((opt = getopt(argc, argv, "s:p:")) != -1) {
		switch (opt) {
			case 's':
				server = optarg;
				break;
			case 'p':
				port = optarg;
				break;
		}
	}

	if (!server || !port) {
		print_usage(argv[0]);
		return EXIT_FAILURE;
	} else {
		io(server, port);
	}

	return EXIT_SUCCESS;
}


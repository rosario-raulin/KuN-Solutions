#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <poll.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "../common/simplesocket.h"
#include "../common/fds.h"
#include "../common/buffer.h"

#define BUFSIZE 8192
#define MAX_REQ_LEN (8192 * 10)
#define DEFAULT_PORT "8080"

static bool verbose = false;

static buffer*
create_response(int status_code, char* status_msg,
	char* payload, int payload_len) {

	buffer* resp = buffer_create();
	resp = buffer_cat_s_n(resp, "HTTP/1.0 ", sizeof("HTTP/1.0 "));
	resp = buffer_cat_i(resp, status_code);
	resp = buffer_cat_s_n(resp, " ", sizeof(" "));
	resp = buffer_cat_s(resp, status_msg);
	resp = buffer_cat_s(resp, "\r\nConnection: close\r\nContent-Length: ");
	resp = buffer_cat_i(resp, payload_len);
	resp = buffer_cat_s_n(resp, "\r\n\r\n", sizeof("\r\n\r\n"));
	resp = buffer_cat_s_n(resp, payload, payload_len);

	return resp;
}

#define PAYLOAD "<html><p>Hello</p></html>"

static void
answer(int to, buffer* b, int end) {
	buffer* resp = create_response(200, "OK", PAYLOAD, sizeof(PAYLOAD));

	if (resp) {
		if (verbose) {
			printf("<< in:\n%s\n", b->p);
			printf(">> out:\n%s\n", resp->p);
		}
		write(to, resp->p, resp->len);
		buffer_free(resp);
	}
}

static char*
header_end(char *buf) {
	return strstr(buf, "\r\n\r\n");
}

static void
handle_request(int fd) {
	buffer* b = buffer_create();

	char buf[BUFSIZE];
	char* end;
	while (b && (end = header_end(b->p)) == NULL && b->size < MAX_REQ_LEN) {
		int bytes_read = read(fd, buf, BUFSIZE);
		if (bytes_read <= 0) {
			buffer_free(b);
			return;
		} else {
			b = buffer_cat_s_n(b, buf, bytes_read);
		}
	}

	if (b) {
		if (end) {
			answer(fd, b, end - b->p);
		} else if (b->size >= MAX_REQ_LEN) {
			puts("req too long");
		}
		buffer_free(b);
	}
}

static fds*
handle_input(fds* f, int pos) {
	const	int fd = f->p[pos].fd;
	if (pos == 0) {
		int next = accept(fd, NULL, NULL);
		if (next == -1) {
			if (verbose) {
				fprintf(stderr, "warning: accept() failed: %s\n", strerror(errno));
			}
		} else {
			f = fds_add(f, next, POLLIN);
		}
	} else {
		handle_request(fd);
		close(fd);
		f = fds_remove(f, pos);
	}
	return f;
}

static void
handle_clients(int s) {
	fds* f = fds_create();

	if ((f = fds_add(f, s, POLLIN)) == NULL) {
		fprintf(stderr, "error: insufficient memory!\n");
		close(s);
		exit(EXIT_FAILURE);
	}

	int revs;
	while ((revs = poll(f->p, f->size, -1)) > 0) {
		int i;
		int handled;
		for (i = handled = 0; i < f->size && handled < revs && f != NULL; ++i) {
			switch (f->p[i].revents) {
				case POLLIN:
					++handled;
					f = handle_input(f, i);
					break;

				case 0:
					break;

				default:
					++handled;
					if (verbose) {
						fprintf(stderr, "warning: unknown revent occured: %d\n",
							f->p[i].revents);
					}
					break;
			}
		}
		if (f == NULL) {
			fprintf(stderr, "error: insufficient memory!\n");
			exit(EXIT_SUCCESS);
		}
	}
}

int
main(int argc, char* argv[]) {
	bool use_v6 = false;
	char* port = DEFAULT_PORT;

	int opt;
	while ((opt = getopt(argc, argv, "p:v6")) != -1) {
		switch (opt) {
			case 'p':
				port = optarg; 
				break;
			case 'v':
				verbose = true;
				break;
			case '6':
				use_v6 = true;
				break;
		}
	}

	const int s = use_v6 ? ssock_v6(port) : ssock_v4(port);
	if (s == -1) {
		fprintf(stderr, "error: %s\n", SOCK_ERR);
		return EXIT_FAILURE;
	}

	handle_clients(s);

	return EXIT_SUCCESS;
}


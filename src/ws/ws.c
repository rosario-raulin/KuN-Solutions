#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <poll.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

#include "../common/simplesocket.h"
#include "../common/fds.h"
#include "../common/buffer.h"
#include "request.h"

#define BUFSIZE 8192 
#define MAX_PATH_LEN 128
#define MAX_REQ_LEN (8192 * 10)
#define DEFAULT_PORT "8080"

#define ERROR_404 -1
#define ERROR_500 -2

static bool verbose = false;

static buffer*
create_response(int status_code, char* status_msg, int payload_len) {

	buffer* resp = buffer_create();
	resp = buffer_cat_s_n(resp, "HTTP/1.0 ", sizeof("HTTP/1.0 "), true);
	resp = buffer_cat_i(resp, status_code);
	resp = buffer_cat_s_n(resp, " ", sizeof(" "), true);
	resp = buffer_cat_s(resp, status_msg);
	resp = buffer_cat_s(resp, "\r\nConnection: close\r\nContent-Length: ");
	resp = buffer_cat_i(resp, payload_len);
	resp = buffer_cat_s_n(resp, "\r\n\r\n", sizeof("\r\n\r\n"), true);

	return resp;
}

static int
get_payload_fd(int* fd, char* path) {

	/* We serve regular files only, so check this first */
	struct stat s;
	if (stat(path, &s) == -1 || !S_ISREG(s.st_mode)) {
		if (verbose) {
			fprintf(stderr, "warning: stat() failed or no regular file!\n");
		}
		return ERROR_404;
	}

	*fd = open(path, O_RDONLY);
	if (*fd == -1) {
		if (verbose) {
			fprintf(stderr, "warning: %s\n", strerror(errno));
		}
		return ERROR_404;
	} else {
		return s.st_size;
	}
}

static void
answer(int to, request* req) {
	if (req) {
		int fd = -1;
		int f_len = get_payload_fd(&fd, req->resource);
			
		buffer* resp;
		switch (f_len) {
			case ERROR_404:
				resp = create_response(404, "Not Found", 0);
				break;
			case ERROR_500:
				resp = create_response(500, "Internal Server Error", 0);
				break;
			default:
				resp = create_response(200, "OK", f_len);
				break;
		}

		if (!resp) {
			fprintf(stderr, "error: insufficient memory!\n");
		} else if (write(to, resp->p, resp->len) != -1 && fd >= 0) {
			char buf[BUFSIZE];
			int bytes_read;
			while ((bytes_read = read(fd, buf, BUFSIZE)) > 0) {
				if (write(to, buf, bytes_read) == -1) break;
			}
		}
		if (fd >= 0) {
			close(fd);
		}
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
	while (b && (end = header_end(b->p)) == NULL && b->len < MAX_REQ_LEN) {
		int bytes_read = read(fd, buf, BUFSIZE);
		if (bytes_read <= 0) {
			buffer_free(b);
			return;
		} else {
			b = buffer_cat_s_n(b, buf, bytes_read, true);
		}
	}

	if (b) {
		if (end) {
			b->p[end - b->p] = '\0';
			request *r = request_create(b->p);
			answer(fd, r);
			request_free(r);
		} else if (b->len >= MAX_REQ_LEN) {
			if (verbose) {
				fprintf(stderr, "warning: request too long!\n");
			}
			buffer* resp = create_response(400, "Bad Request", 0);
			if (resp) {
				write(fd, resp->p, resp->len);
				buffer_free(resp);
			}
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
handle_clients_fork(int s) {
	while (true) {
		int fd = accept(s, NULL, NULL);
		if (fd != -1) {
			switch (fork()) {
				case 0:
					handle_request(fd);
					close(fd);
					exit(EXIT_SUCCESS);
				case -1:
					fprintf(stderr, "error: fork() failed!\n");
					exit(EXIT_FAILURE);
					break;
				default:
					close(fd);
					break;
			}
		}
	}
}

static void
handle_clients_poll(int s) {
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
	bool use_fork = false;
	bool use_chroot = false;
	char* port = DEFAULT_PORT;

	int opt;
	while ((opt = getopt(argc, argv, "cfv6p:")) != -1) {
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
			case 'f':
				use_fork = true;
				break;
			case 'c':
				use_chroot = true;
				break;
		}
	}
	
	struct sigaction act;
	act.sa_handler = SIG_IGN;
	if (sigaction(SIGPIPE, &act, NULL) == -1) {
		fprintf(stderr, "error: changing signaler handlers failed!\n");
		return EXIT_FAILURE;
	}

	if (use_chroot) {
		char wd[MAX_PATH_LEN];
		if (getcwd(wd, MAX_PATH_LEN) == NULL || chdir(wd) == -1 || chroot(wd) == -1) {
			fprintf(stderr, "error: chrooting didn't work, please run as root!\n");
			return EXIT_FAILURE;
		}
	}

	const int s = use_v6 ? ssock_v6(port) : ssock_v4(port);
	if (s == -1) {
		fprintf(stderr, "error: %s\n", SOCK_ERR);
		return EXIT_FAILURE;
	}

	if (use_fork) {
		handle_clients_fork(s);
	} else {
		handle_clients_poll(s);
	}

	return EXIT_SUCCESS;
}


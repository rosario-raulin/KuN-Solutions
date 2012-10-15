#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

extern char* SOCK_ERR;

static struct addrinfo*
get_addr_info(char *host, char *port, struct addrinfo** res, int family) {
	struct addrinfo hints;

	hints.ai_family = family;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_protocol = 0;
	hints.ai_next = NULL;
	hints.ai_addr = NULL;
	hints.ai_canonname = NULL;

	int gai_ret;
	if ((gai_ret = getaddrinfo(host, port, &hints, res)) != 0) {
		SOCK_ERR = (char*) gai_strerror(gai_ret);
		return NULL;
	}

	return *res;
}

static int
get_socket(char* host, char* port, int fam,
		int (*socket_fn)(int, const struct sockaddr*, socklen_t)) {
	struct addrinfo *res;
	res = get_addr_info(host, port, &res, fam);
	if (res == NULL) { return -1; }

	int s;
	struct addrinfo *rp;
	for (rp = res; rp != NULL; rp = rp->ai_next) {
		s = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (s == -1) { continue; }
		if (socket_fn(s, rp->ai_addr, rp->ai_addrlen) == 0) { break; }
	}

	if (rp == NULL) {
		SOCK_ERR = "could not bind/connect to any host";
		return -1;
	}

	freeaddrinfo(res);
	return s;

}

int
csock(char* host, char* port) {
	return get_socket(host, port, AF_UNSPEC, &connect);
}

static int
ssock(int fam, char* port) {
	int s = get_socket(NULL, port, fam, &bind);
	if (s != -1 && listen(s, 3) < 0) {
		s = -1;
	}
	return s;
}

int
ssock_v4(char* port) {
	return ssock(AF_INET, port);
}

int
ssock_v6(char* port) {
	return ssock(AF_INET6, port);
}


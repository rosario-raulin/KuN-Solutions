#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

extern char* SOCK_ERR;

int
csock_v4(char* host, char* port) {

	struct addrinfo hints;
	struct addrinfo *res;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_protocol = 0;
	hints.ai_next = NULL;
	hints.ai_addr = NULL;
	hints.ai_canonname = NULL;
	
	int gai_ret;
	if ((gai_ret = getaddrinfo(host, port, &hints, &res)) != 0) {
		SOCK_ERR = (char*) gai_strerror(gai_ret);
		return -1;
	}

	int s;
	struct addrinfo *rp;
	for (rp = res; rp != NULL; rp = rp->ai_next) {
		s = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (s == -1) { continue; }
		if (connect(s, rp->ai_addr, rp->ai_addrlen) == 0) {	break; }
	}

	if (rp == NULL) {
		SOCK_ERR = "could not connect to any host";
		return -1;
	}

	freeaddrinfo(res);
	return s;
}


typedef struct fds {
	int size;
	int capacity;
	struct pollfd *p;
} fds;

fds* fds_create();
fds* fds_add(fds* f, int s, int events);
fds* fds_remove(fds* f, int i);


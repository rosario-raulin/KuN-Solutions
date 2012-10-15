#include <stdlib.h>
#include <unistd.h>
#include <poll.h>

#include "fds.h"

#define DEFAULT_CAPACITY 8192

fds*
fds_create() {
	fds* f = malloc(sizeof(struct fds));
	if (f) {
		f->size = 0;
		f->capacity = DEFAULT_CAPACITY;
		f->p = malloc(f->capacity * sizeof(struct pollfd));
		if (!f->p) {
			free(f);
			f = NULL;
		}
	}
	return f;
}

fds*
fds_add(fds* f, int s, int events) {
	if (f) {
		if (f->size == f->capacity) {
			f->capacity *= 2;
			f->p = realloc(f->p, f->capacity * sizeof(struct pollfd));
			if (!f->p) {
				free(f);
				f = NULL;
			}
		}
		f->p[f->size].fd = s;
		f->p[f->size++].events = events;
	}
	return f;
}

fds*
fds_remove(fds* f, int i) {
	if (f && i <= f->size) {
		close(f->p[i].fd);
		f->p[i] = f->p[--f->size];
	}
	return f;
}

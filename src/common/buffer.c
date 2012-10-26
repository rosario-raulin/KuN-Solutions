#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "buffer.h"

#define MAX_NUMBER_LEN sizeof("4294967296")
#define BUFFER_DEFAULT_SIZE 8192

buffer*
buffer_create() {
	buffer* b = malloc(sizeof(struct buffer));
	if (b) {
		b->len = 0;
		b->size = BUFFER_DEFAULT_SIZE;
		b->p = malloc(b->size);
		if (!b->p) {
			free(b);
			b = NULL;
		}
	}
	return b;
}

static buffer*
buffer_extend(buffer* b, int add) {
	if (b && (b->len + add) >= b->size) {
		b->size += (b->len + add) * 2;
		b->p = realloc(b->p, b->size);
		if (!b->p) {
			free(b);
			b = NULL;
		}
	}
	return b;
}

buffer*
buffer_cat_s_n(buffer* b, char* s, int len, bool terminate) {
	if (s[len-1] == '\0') { /* TODO: This should not be needed...  */
		--len;
	}
	b = buffer_extend(b, len);
	if (b) {
		memcpy(b->p + b->len, s, len);
		b->len += len;
		if (terminate) {
			b->p[b->len] = '\0';
		}	
	}

	return b;
}

buffer*
buffer_cat_s(buffer* b, char* s) {
	return buffer_cat_s_n(b, s, strlen(s), true);
}

buffer*
buffer_cat_i(buffer* b, int i) {
	char buf[MAX_NUMBER_LEN];
	int copied;

	if ((copied = snprintf(buf, MAX_NUMBER_LEN, "%d", i)) > 0) {
		b = buffer_cat_s_n(b, buf, copied, true);
	} else {
		b = NULL;
	}
	return b;
}

void
buffer_free(buffer* b) {
	if (b) {
		free(b->p);
		free(b);
	}
}

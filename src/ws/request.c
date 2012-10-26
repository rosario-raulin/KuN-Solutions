#include <stdlib.h>
#include <string.h>

#include "request.h"

// TODO: add real request parsing
request*
request_create(char* input) {
	request *r = malloc(sizeof(struct request));
	if (r) {
		char* s = strtok(input, "\r\n");
		if (s) {
			char* method = strtok(s, " ");
			char* resource = strtok(NULL, " ");
			char* version = strtok(NULL, " ");

			if (method && version && resource) {
				r->type = GET;
				if (strcmp(resource, "/") == 0) {
					resource = "/index.html";
				}
				r->resource = ++resource;
				r->version = ZERO;
			} else {
				free(r);
				r = NULL;
			}
		}
	}
	return r;
}

void
request_free(request* r) {
	if (r) {
		free(r);
	}
}


#include <stdlib.h>

#include "htable.h"

htable*
htable_create(int size) {
	htable* t = malloc(sizeof(struct htable));
	if (t) {
		t->size = size;
		t->nelements = 0;
		t->e = malloc(sizeof(hentry*) * t->size);
		if (!t->e) {
			free(t);
			t = NULL;
		}
	}

	return t;
}

htable*
htable_add(htable* t, int key, void* value) {
	hentry *dest = t->e[key % t->size];
	if (dest) {
		while (dest->key != key && dest->next != NULL) {
			dest = dest->next;
		}
		if (dest->key == key) { /* Override value */
			dest->value = value;
			return t;
		}
	}

	/* Okay, we have to create a new entry */
	hentry* next = malloc(sizeof(struct hentry));
	if (!next) { return NULL; }
	next->key = key;
	next->value = value;
	next->next = NULL;

	if (dest) {
		dest->next = next;
	} else {
		t->e[key % t->size] = next;
	}

	++t->nelements;

	return t;
}

void*
htable_get(htable* t, int key) {
	hentry *e = t->e[key % t->size];
	while (e && e->key != key) {
		e = e->next;
	}
	return e == NULL ? NULL : e->value;
}

void
htable_free(htable* t) {
	int i;
	for (i = 0; i < t->size; ++i) {
		hentry* curr = t->e[i];
		hentry* next;
		while (curr) {
			next = curr->next;
			free(curr);
			curr = next;
		}
	}
	free(t->e);
	free(t);
}


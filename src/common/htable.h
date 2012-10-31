typedef struct hentry {
	int key;
	void* value;
	struct hentry* next;
} hentry;

typedef struct htable {
	int size;
	int nelements;
	hentry** e;
} htable;

htable* htable_create(int size);
htable* htable_add(htable* t, int key, void* value);
void* htable_get(htable* t, int key);
void htable_free(htable* t);


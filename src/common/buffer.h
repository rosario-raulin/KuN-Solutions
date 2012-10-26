typedef struct buffer {
	int len;
	int size;
	char* p;
} buffer;

buffer* buffer_create();
buffer* buffer_cat_s_n(buffer* b, char* s, int len);
buffer* buffer_cat_s(buffer* b, char* s);
buffer* buffer_cat_i(buffer* b, int i);
void buffer_free(buffer* b);


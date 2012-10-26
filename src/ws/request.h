typedef enum request_type
	{ GET, POST, HEAD, PUT,
		DELETE, TRACE, OPTIONS, CONNECT }
request_type;

typedef enum http_version
	{	ZERO,	ONE }
http_version;

typedef struct request {
	request_type type;
	char* resource;
	http_version version;
} request;

request* request_create(char* input);
void request_free(request* r);

#ifndef __HTTP_HH__
#define __HTTP_HH__

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>

typedef const char *http_code_t;
typedef const char *http_type_t;

static const char *HTTP_STATUS_200 = "200 OK";
static const char *HTTP_STATUS_404 = "404 Not Found";
static const char *HTTP_STATUS_500 = "500 Internal Server Error";

static const char *HTTP_TYPE_HTML = "text/html";
static const char *HTTP_TYPE_PLAIN = "text/plain";
static const char *HTTP_TYPE_JSON = "application/json";
static const char *HTTP_TYPE_CSS = "text/css";
static const char *HTTP_TYPE_JS = "application/javascript";
static const char *HTTP_TYPE_PNG = "image/png";
static const char *HTTP_TYPE_JPEG = "image/jpeg";
static const char *HTTP_TYPE_GIF = "image/gif";

char *get_http_req_url(const char *request);
void handle_http_request(int connection_fd);
char *make_http_response(const char *content, const http_code_t code, const http_type_t type);
char *read_file_content(const char *file_path);

#endif // __HTTP_HH__
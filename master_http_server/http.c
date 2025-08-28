#include "http.h"

char *get_http_req_url(const char *request)
{
    // 假设请求格式为 "GET /path HTTP/1.1"
    const char *start = strstr(request, "GET ");
    if (!start)
        return NULL;

    start += 4; // 跳过 "GET "
    const char *end = strstr(start, " HTTP/");
    if (!end)
        return NULL;

    size_t length = end - start;
    char *url = malloc(length + 1);
    if (!url)
        return NULL;

    strncpy(url, start, length);
    url[length] = '\0'; // 确保字符串结束
    return url;
}

char *make_http_response(const char *content, const http_code_t code, const http_type_t type)
{
    // 简单的HTTP响应格式
    const char *response_template = "HTTP/1.1 %s\r\n"
                                    "Content-Type: %s\r\n"
                                    "Content-Length: %zu\r\n"
                                    "\r\n"
                                    "%s";
    size_t content_length = strlen(content);
    size_t response_size = strlen(response_template) + strlen(code) + strlen(type) + content_length + 20; // 20用于数字长度和其他字符
    char *response = malloc(response_size);
    if (!response)
        return NULL;
    snprintf(response, response_size, response_template, code, type, content_length, content);
    return response;
}

void handle_http_request(int connection_fd)
{
    // printf("Client connected\n");
    char buffer[1024 * 8] = {0};
    ssize_t bytes_received = recv(connection_fd, buffer, sizeof(buffer) - 1, 0);
    buffer[bytes_received] = '\0'; // 确保字符串结束

    char *url = get_http_req_url(buffer);
    if (url)
    {
        // printf("Received URL: %s\n", url);
        char *file_content = read_file_content(url);
        char *response;
        if (file_content)
        {
            response = make_http_response(file_content, HTTP_STATUS_200, HTTP_TYPE_HTML);
        }
        else
        {
            response = make_http_response("File not found", HTTP_STATUS_404, HTTP_TYPE_PLAIN);
        }
        if (response)
        {
            send(connection_fd, response, strlen(response), 0);
            free(response);
        }
        free(file_content);
        free(url);
    }
    close(connection_fd);
}

char *read_file_content(const char *file_path)
{
    char file_path_buffer[strlen(file_path) + 1];
    sprintf(file_path_buffer, ".%s", file_path);

    FILE *file = fopen(file_path_buffer, "r");
    if (!file)
    {
        perror("Failed to open file");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *content = malloc(file_size + 1);
    if (!content)
    {
        fclose(file);
        perror("Failed to allocate memory for file content");
        return NULL;
    }

    fread(content, 1, file_size, file);
    content[file_size] = '\0'; // 确保字符串结束
    fclose(file);
    return content;
}
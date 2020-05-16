#include "http_request_parser.h"
#include <string.h>

http_request_parser http_request_parser_init(size_t num_lines)
{
    http_request_parser result = (http_request_parser)malloc(sizeof(struct http_request_parser_struct));
    result->lines = (char**)malloc(sizeof(char*) * num_lines);
    for(size_t i = 0; i < num_lines; i++) {
        result->lines[i] = (char*)malloc(sizeof(char));
        result->lines[i][0] = '\0';
    }
    result->num_lines = num_lines;
    return result;
}

int http_request_parse(http_request_parser request, const char *data)
{
    const char *index = data;
    size_t line = 0;
    while(strchr(request->lines[line], '\n') != NULL) {
        line++;
        if(line == request->num_lines) return -1;
    }
    
    char *endline = strchr(index, '\n');
    while(endline != NULL) {
        const size_t length = endline - index + 1;
        char *current = request->lines[line];
        const size_t current_length = strlen(current);
        char *next = (char*)malloc(sizeof(char) * (current_length + length + 1));
        memcpy(next, current, current_length);
        memcpy(next + current_length, index, length);
        next[current_length + length] = '\0';
        request->lines[line] = next;
        free(current);
        index += length;
        line++;
        if(line == request->num_lines) return -1;
        endline = strchr(index, '\n');
    }

    return index - data;
}

void http_request_parser_destroy(http_request_parser request)
{
    for(size_t i = 0; i < request->num_lines; i++) {
        free(request->lines[i]);
    }
    free(request);
}
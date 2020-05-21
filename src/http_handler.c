#include "http_handler.h"

http_handler_t http_handler_init(const char *filter, http_transaction_callback *callback, void* context)
{
    http_handler_t result = (http_handler_t)malloc(sizeof(struct http_handler_struct));
    result->filter = (char*)malloc((strlen(filter) + 1) * sizeof(char));
    strcpy(result->filter, filter);
    result->callback = callback;
    result->context = context;
    result->next = NULL;
    return result;
}

void http_handler_add(http_handler_t *first, http_handler_t addition)
{
    if(*first == NULL) {
        *first = addition;
        return;
    }

    http_handler_t current = *first;
    http_handler_t next = NULL;
    while(current->next != NULL) {
        next = current->next;
        current = next;
    }
    current->next = addition;
}

void http_handler_remove(http_handler_t *first, http_handler_t removal)
{
    if(*first == NULL) return;
    if(*first == removal) {
        *first = NULL;
        return;
    }
    if(first[0]->next == NULL) return;
    http_handler_t current = first[0]->next;
    http_handler_t parent = *first;
    http_handler_t next = NULL;
    while(current != NULL) {
        if(current == removal) {
            parent->next = current->next;
            return;
        } else {
            next = current->next;
            parent = current;
            current = next;
        }
    }
}

bool http_handler_execute(http_handler_t first, http_transaction_t transaction)
{
    http_handler_t current = first;
    http_handler_t next = NULL;
    while(current != NULL) {
        if(http_path_filter(transaction->request->resource->path, current->filter)) {
            current->callback(transaction, current->context);
            return true;
        } else {
            next = current->next;
            current = next;
        }
    }
    return false;
}

http_handler_t http_handler_destroy(http_handler_t handler)
{
    if(handler == NULL) return NULL;
    http_handler_t result = handler->next;
    free(handler->filter);
    free(handler);
    return result;
}
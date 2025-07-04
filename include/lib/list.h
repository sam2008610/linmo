/* Singly-linked list
 * – Two sentinel nodes (head and tail) eliminate edge-case tests.
 * – The list stores generic data pointers ('void *').
 * – All primitives are defined `static inline` so they can live entirely
 *   in the header without multiple-definition issues.
 * – Comments follow C style and use concise American English.
 */

#pragma once

#include <lib/libc.h>
#include <lib/malloc.h>

/* List node */
typedef struct list_node {
    struct list_node *next;
    void *data;
} list_node_t;

/* Public list descriptor */
typedef struct {
    list_node_t *head; /* dummy head sentinel (always non-NULL) */
    list_node_t *tail; /* dummy tail sentinel (always last) */
    size_t length;     /* number of data nodes */
} list_t;

static inline list_t *list_create(void)
{
    list_t *list = malloc(sizeof(list_t));
    list_node_t *head = malloc(sizeof(list_node_t));
    list_node_t *tail = malloc(sizeof(list_node_t));

    if (!list || !head || !tail) { /* cleanup on failure */
        free(tail);
        free(head);
        free(list);
        return NULL;
    }

    head->next = tail;
    head->data = NULL;

    tail->next = NULL;
    tail->data = NULL;

    list->head = head;
    list->tail = tail;
    list->length = 0U;
    return list;
}

static inline int list_is_empty(const list_t *list)
{
    return !list || list->length == 0U;
}

/* Safe single-step successor: returns NULL for tail or NULL input */
static inline list_node_t *list_next(const list_node_t *node)
{
    return (node && node->next) ? node->next : NULL;
}

/* Circular next: wraps from tail sentinel to first data node */
static inline list_node_t *list_cnext(const list_t *list,
                                      const list_node_t *node)
{
    if (!list || !node)
        return NULL;
    return (node->next == list->tail) ? list->head->next : node->next;
}

/* Push and pop */

static inline list_node_t *list_pushback(list_t *list, void *data)
{
    if (!list)
        return NULL;

    list_node_t *node = malloc(sizeof(list_node_t));
    if (!node)
        return NULL;

    node->data = data;
    node->next = list->tail;
    list->tail->data = NULL; /* tail sentinel never holds data */
    list->head->data = NULL;

    /* Insert before tail sentinel */
    list_node_t *prev = list->head;
    while (prev->next != list->tail)
        prev = prev->next;
    prev->next = node;

    list->length++;
    return node;
}

static inline void *list_pop(list_t *list)
{
    if (list_is_empty(list))
        return NULL;

    list_node_t *first = list->head->next;
    list->head->next = first->next;

    void *data = first->data;
    free(first);
    list->length--;
    return data;
}

/* Remove a specific node; returns its data */
static inline void *list_remove(list_t *list, list_node_t *target)
{
    if (!list || !target || list_is_empty(list))
        return NULL;

    list_node_t *prev = list->head;
    while (prev->next != list->tail && prev->next != target)
        prev = prev->next;

    if (prev->next != target)
        return NULL; /* node not found */

    prev->next = target->next;
    void *data = target->data;
    free(target);
    list->length--;
    return data;
}

/* Iteration */

/* Callback should return non-NULL to stop early, NULL to continue */
static inline list_node_t *list_foreach(list_t *list,
                                        list_node_t *(*cb)(list_node_t *,
                                                           void *),
                                        void *arg)
{
    if (!list || !cb)
        return NULL;

    list_node_t *node = list->head->next;
    while (node != list->tail) {
        list_node_t *res = cb(node, arg);
        if (res)
            return res;
        node = node->next;
    }
    return NULL;
}

/* Clear and destroy */

static inline void list_clear(list_t *list)
{
    if (!list)
        return;
    while (!list_is_empty(list))
        list_pop(list);
}

static inline void list_destroy(list_t *list)
{
    if (!list)
        return;
    list_clear(list);
    free(list->head);
    free(list->tail);
    free(list);
}

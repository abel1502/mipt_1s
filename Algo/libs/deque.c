#include <stdlib.h>
#include <stdio.h>
#include <assert.h>


typedef unsigned int item_t;

static const size_t MAX_DEQUE = (size_t)-1;

typedef struct deque_item_s deque_item;
typedef struct deque_s deque;


struct deque_item_s {
    item_t val;
    deque_item *next;
    deque_item *prev;
};

struct deque_s {
    deque_item *front;
    deque_item *back;
    size_t size;
};

/*

I feel too lazy to implement a proper abstract dequeue, so this one will:
 - lack error codes
 - lack peek methods
 - assume that item_t is small enough too copy rather than pass by pointer

Hopefully this is fine

*/

deque *deque_new();
void deque_delete(deque *self);
void deque_pushFront(deque *self, item_t value);
void deque_pushBack(deque *self, item_t value);
item_t deque_popFront(deque *self);
item_t deque_popBack(deque *self);
void deque_clear(deque *self);
void deque_dump(deque *self);


deque *deque_new() {
    deque *self = (deque *)malloc(sizeof(deque));
    if (self == NULL) {
        return NULL;
    }

    self->size = 0;
    self->front = NULL;
    self->back = NULL;

    return self;
}

void deque_delete(deque *self) {
    if (self == NULL)
        return;

    deque_clear(self);

    free(self);
}

void deque_pushFront(deque *self, item_t value) {
    assert(self != NULL);
    assert(self->size < MAX_DEQUE);

    deque_item *newItem = (deque_item *)malloc(sizeof(deque_item));
    assert(newItem != NULL);

    newItem->val = value;
    newItem->next = NULL;
    newItem->prev = self->front;

    if (self->front != NULL) {
        self->front->next = newItem;
    }

    self->front = newItem;

    if (self->back == NULL) {
        self->back = newItem;
    }

    self->size++;
}

void deque_pushBack(deque *self, item_t value) {
    assert(self != NULL);
    assert(self->size < MAX_DEQUE);

    deque_item *newItem = (deque_item *)malloc(sizeof(deque_item));
    assert(newItem != NULL);

    newItem->val = value;
    newItem->next = self->back;
    newItem->prev = NULL;

    if (self->back != NULL) {
        self->back->prev = newItem;
    }

    self->back = newItem;

    if (self->front == NULL) {
        self->front = newItem;
    }

    self->size++;
}

item_t deque_popFront(deque *self) {
    assert(self != NULL);
    assert(self->front != NULL);

    deque_item *oldItem = self->front;
    self->front = oldItem->prev;

    if (self->front != NULL) {
        self->front->next = NULL;
    }

    if (self->back == oldItem) {
        self->back = NULL;
    }

    item_t value = oldItem->val;

    self->size--;

    free(oldItem);

    return value;
}

item_t deque_popBack(deque *self) {
    assert(self != NULL);
    assert(self->back != NULL);

    deque_item *oldItem = self->back;
    self->back = oldItem->next;

    if (self->back != NULL) {
        self->back->prev = NULL;
    }

    if (self->front == oldItem) {
        self->front = NULL;
    }

    item_t value = oldItem->val;

    self->size--;

    free(oldItem);

    return value;
}

void deque_clear(deque *self) {
    assert(self != NULL);

    while (self->size != 0) {
        deque_popFront(self);
    }
}

void deque_dump(deque *self) {
    printf("deque { [%p]\n", self);
    printf("  size  = %zu\n", self->size);
    printf("  front = [%p]\n", self->front);
    printf("  back  = [%p]\n", self->back);
    printf("  data {\n");

    size_t cnt = 0;
    for (deque_item *cur = self->back; cur != NULL; cur = cur->next) {
        printf("    [%p] = ", cur);
        if (cur != NULL)
            printf("%u\n", cur->val);
        else
            printf("<?>\n");

        cnt++;
        if (cnt > self->size + 10) {
            break;
        }
    }

    printf("  }\n");
    printf("}\n");
}

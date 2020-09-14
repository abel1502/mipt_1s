//#define DBG

#include <stdlib.h>

#ifdef DBG
#include <stdio.h>

typedef enum stack_code_e { STACK_OK, STACK_NULL, STACK_OVERFLOW, STACK_UNDERFLOW } stack_code;
#endif

typedef struct {
    size_t max_size;
    int * top;
    int * values;
} stack;

stack * stack_new(size_t size) {
    stack * st = (stack *)malloc(sizeof(stack));
    if (st == NULL) {
        return NULL;
    }
    st->max_size = size;
    st->top = st->values = (int *)calloc(size, sizeof(int));
    if (st->values == NULL) {
        free(st);
        return NULL;
    }
    return st;
}

stack_code stack_delete(stack *st) {
    if (st == NULL) {
        return STACK_NULL;
    }
    if (st->values == NULL) {
        free(st);
        return STACK_NULL;
    }
    free(st->values);
    free(st);
    return STACK_OK;
}

stack_code stack_push(stack *st, int value) {
    if (st == NULL || st->values == NULL || st->top == NULL) {
        return STACK_NULL;
    }
    if (st->top >= st->values + st->max_size) {
        return STACK_OVERFLOW;
    }
    *(st->top++) = value;
    return STACK_OK;
}

stack_code stack_pop(stack *st, int *value) {
    if (st == NULL || st->values == NULL || st->top == NULL || value == NULL) {
        return STACK_NULL;
    }
    if (st->top <= st->values) {
        return STACK_UNDERFLOW;
    }
    *value = *(--st->top);
    return STACK_OK;
}

#ifdef DBG
int main() {
    stack * st = stack_new(10);
    stack_push(st, 123);
    int value;
    stack_pop(st, &value);
    printf("%d", value);

    return 0;
}
#endif

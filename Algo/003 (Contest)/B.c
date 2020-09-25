/*

По заданной строке из круглых/квадратных/фигурных открывающих и закрывающих скобок определить, является ли она правильной скобочной последовательностью.

Множество правильных скобочных последовательностей (ПСП) определяется как наименьшее множество с условиями:

пустая строка является ПСП;
если S — ПСП, то (S), [S], {S} — тоже ПСП;
если S1 и S2 — ПСП, то S1S2 — тоже ПСП.
Входные данные
В единственной строке содержится последовательность из круглых/квадратных/фигурных открывающих и закрывающих скобок.

Выходные данные
Выведите «yes», если введённая строка является правильной скобочной последовательностью, и «no» иначе.

Примеры
входные данные
()[]
выходные данные
yes
входные данные
([)]
выходные данные
no

*/

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>


typedef char item_t;
typedef struct stack_item_s stack_item;
typedef struct stack_s stack;


const size_t MAX_STACK = (size_t)-1;


struct stack_item_s {
    item_t val;
    stack_item *prev;
};

struct stack_s {
    stack_item *top;
    size_t size;
};

typedef enum {
    STACK_OK,
    STACK_NULL,
    STACK_OVERFLOW,
    STACK_UNDERFLOW
} stack_code;


stack *stack_new();
stack_code stack_delete(stack *st);
stack_code stack_push(stack *st, item_t value);
stack_code stack_pop(stack *st, item_t *value);
stack_code stack_peek(stack *st, item_t *value);
stack_code stack_clear(stack *st);


stack *stack_new() {
    stack * st = (stack *)malloc(sizeof(stack));
    if (st == NULL) {
        return NULL;
    }
    st->size = 0;
    st->top = NULL;
    return st;
}

stack_code stack_delete(stack *st) {
    if (st == NULL) {
        return STACK_NULL;
    }
    stack_code res;  // For some reason inline declaration didn't work
    if ((res = stack_clear(st)) != STACK_OK) {
        return res;
    }
    free(st);
    return STACK_OK;
}

stack_code stack_push(stack *st, item_t value) {
    if (st == NULL) {
        return STACK_NULL;
    }
    if (st->size >= MAX_STACK) {
        return STACK_OVERFLOW;
    }

    stack_item *newTop = (stack_item *)malloc(sizeof(stack_item));
    if (newTop == NULL) {
        return STACK_OVERFLOW;
    }

    newTop->prev = st->top;
    newTop->val = value;

    st->top = newTop;
    assert(st->size < MAX_STACK);
    st->size++;

    return STACK_OK;
}

stack_code stack_pop(stack *st, item_t *value) {
    if (st == NULL || value == NULL) {
        return STACK_NULL;
    }
    if (st->top == NULL) {
        return STACK_UNDERFLOW;
    }

    stack_item *oldTop = st->top;

    *value = oldTop->val;

    st->top = oldTop->prev;
    assert(st->size > 0);
    st->size--;
    free(oldTop);

    return STACK_OK;
}

stack_code stack_peek(stack *st, item_t *value) {
    if (st == NULL || value == NULL) {
        return STACK_NULL;
    }
    if (st->top == NULL) {
        return STACK_UNDERFLOW;
    }

    *value = st->top->val;

    return STACK_OK;
}

stack_code stack_clear(stack *st) {
    if (st == NULL) {
        return STACK_NULL;
    }

    item_t tmp;  // = 0; // Can't be initialized because item_t in general may be both a struct or a number, for example
    while (st->size > 0) {
        stack_pop(st, &tmp);
    }
    return STACK_OK;
}


/*
 CBS = Correct Bracket Sequence

 Returns 1 if stdin denotes a CBS and 0 otherwise
*/
int checkCBS() {
    int cur = EOF;
    stack *st = stack_new();
    int result = 0;
    item_t tos = 0;  // Top of Stack

    while ((cur = getchar()) != EOF || cur == '\n') {
        switch (cur) {
        case '(':
        case '[':
        case '{':
            stack_push(st, (char)cur);
            break;
        case ')':
            if (stack_pop(st, &tos) != STACK_OK || tos != '(') {
                result = 0;
                goto ret;
            }
            break;
        case ']':
            if (stack_pop(st, &tos) != STACK_OK || tos != '[') {
                result = 0;
                goto ret;
            }
            break;
        case '}':
            if (stack_pop(st, &tos) != STACK_OK || tos != '{') {
                result = 0;
                goto ret;
            }
            break;
        }
    }

    result = (st->size == 0);

ret:
    stack_delete(st);
    return result;
}


int main() {
    if (checkCBS()) {
        printf("yes\n");
    } else {
        printf("no\n");
    }

    return 0;
}

/*

Complexity: Linear because stack)

*/

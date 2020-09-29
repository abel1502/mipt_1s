/*

Гистограмма является многоугольником, сформированным из последовательности прямоугольников, выровненных на общей базовой линии. Прямоугольники имеют равную ширину, но могут иметь различные высоты. Обычно гистограммы используются для представления дискретных распределений, например, частоты символов в текстах. Отметьте, что порядок прямоугольников очень важен. Вычислите область самого большого прямоугольника в гистограмме, который также находится на общей базовой линии.

Входные данные
В первой строке входного файла записано число N (0<N?106)  ? количество прямоугольников гистограммы. Затем следует N целых чисел h1…hn, где 0?hi?109. Эти числа обозначают высоты прямоугольников гистограммы слева направо. Ширина каждого прямоугольника равна 1.

Выходные данные
Выведите площадь самого большого прямоугольника в гистограмме. Помните, что этот прямоугольник должен быть на общей базовой линии.

Примеры
входные данные
7 2 1 4 5 1 3 3
выходные данные
8
входные данные
3 2 1 2
выходные данные
3
входные данные
1 0
выходные данные
0

*/

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>


typedef struct {
    unsigned long long ind;
    unsigned long long val;
} item_t;

typedef struct stack_item_s stack_item;
typedef struct stack_s stack;


static const size_t MAX_STACK = (size_t)1000001;


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

void readItem(item_t *item, unsigned int ind) {
    assert(item != NULL);

    item->ind = ind;
    int res = scanf("%llu", &(item->val));
    assert(res == 1);
}

void popUpdate(stack *st, unsigned long long ind, unsigned long long *bestResult) {
    stack_code res = STACK_OK;

    item_t tos1 = {};

    res = stack_pop(st, &tos1);
    assert(res == STACK_OK);

    item_t tos2 = {};
    res = stack_peek(st, &tos2);
    assert(res != STACK_NULL);

    unsigned long width = res == STACK_UNDERFLOW ? ind : ind - tos2.ind - 1;

    unsigned long long curResult = tos1.val * width;

    if (curResult > *bestResult) {
        *bestResult = curResult;
    }
}

int main() {
    stack_code res = STACK_OK;

    unsigned long long n = 0;
    scanf("%d", &n);

    stack *st = stack_new();

    unsigned long long bestResult = 0;

    item_t tos = {};  // Top of Stack
    item_t cur = {};  // Current
    readItem(&cur, 0);

    for (unsigned long long i = 0; i < n; ) {
        res = stack_peek(st, &tos);
        assert(res != STACK_NULL);
        if (res == STACK_UNDERFLOW || (res == STACK_OK && tos.val <= cur.val)) {
            res = stack_push(st, cur);
            assert(res == STACK_OK);
            i++;
            if (i < n) readItem(&cur, i);
        } else {
            assert(res == STACK_OK);  // Seems to be unconditionally true, but might as check as well

            popUpdate(st, i, &bestResult);
        }
    }

    while (st->size > 0) {
        popUpdate(st, n, &bestResult);
    }

    stack_delete(st);

    printf("%llu", bestResult);

    return 0;
}

/*

Complexity: Linear because everything is push and popped only once

*/

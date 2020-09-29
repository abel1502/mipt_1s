/*

Реализуйте свой стек. Решения, использующие std::stack, получат 1 балл. Решения, хранящие стек в массиве, получат 1.5 балла. Решения, использующие указатели, получат 2 балла.

Гарантируется, что количество элементов в стеке ни в какой момент времени не превышает 10000.

Обработайте следующие запросы:

push n: добавить число n в конец стека и вывести «ok»;
pop: удалить из стека последний элемент и вывести его значение, либо вывести «error», если стек был пуст;
back: сообщить значение последнего элемента стека, либо вывести «error», если стек пуст;
size: вывести количество элементов в стеке;
clear: опустошить стек и вывести «ok»;
exit: вывести «bye» и завершить работу.
Входные данные
В каждой строке входных данных задана одна операция над стеком в формате, описанном выше.

Выходные данные
В ответ на каждую операцию выведите соответствующее сообщение.

Пример
входные данные
push 1
back
exit

выходные данные
ok
1
bye

*/

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>


typedef long long int item_t;
typedef struct stack_item_s stack_item;
typedef struct stack_s stack;


static const size_t MAX_STACK = 10001;      // Generally (size_t)-1, but for this task the limit is explicitly specified

static const size_t MAX_CMD = 6;            // "clear\0"

static const char CMD_FORMAT[] = "%5s";     // Has to be specified explicitly because otherwise I would have had to do
                                            // like sprintf("%%%ds", MAX_CMD - 1);, and that feels overcomplicated

static const char ITEM_FORMAT[] = "%lld";   // Same as above, but here there's no way I'm aware of to retrieve the
                                            // format specifier from the type


struct stack_item_s {
    item_t val;
    stack_item *prev;
};

struct stack_s {
    stack_item *top;
    size_t size;
};

typedef enum {
    STACK_OK,  // Has to be equal to 0 for compact success checks
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

int main() {
    stack *st = stack_new();

    while (1) {
        char cmd[MAX_CMD];
        stack_code res = STACK_OK;

        scanf(CMD_FORMAT, cmd);

        if (strcmp(cmd, "push") == 0) {
            item_t value = 0;
            scanf(ITEM_FORMAT, &value);
            res = stack_push(st, value);
            assert(res == STACK_OK);
            printf("ok\n");
            continue;
        }

        if (strcmp(cmd, "pop") == 0) {
            item_t value = 0;
            res = stack_pop(st, &value);
            if (res == STACK_UNDERFLOW) {
                printf("error\n");
                continue;
            }
            assert(res == STACK_OK);
            printf(ITEM_FORMAT, value);
            printf("\n");
            continue;
        }

        if (strcmp(cmd, "back") == 0) {
            item_t value = 0;
            res = stack_peek(st, &value);
            if (res == STACK_UNDERFLOW) {
                printf("error\n");
                continue;
            }
            assert(res == STACK_OK);
            printf(ITEM_FORMAT, value);
            printf("\n");
            continue;
        }

        if (strcmp(cmd, "size") == 0) {
            printf("%zd\n", st->size);
            continue;
        }

        if (strcmp(cmd, "clear") == 0) {
            res = stack_clear(st);
            assert(res == STACK_OK);
            printf("ok\n");
            continue;
        }

        if (strcmp(cmd, "exit") == 0) {
            printf("bye\n");
            res = stack_delete(st);
            // assert(res == STACK_OK); // Intentionally omitted because cleanup should be performed on the stack in any state, even broken
            return EXIT_SUCCESS;
        }

        assert(0); // Is only reached upon an unknown command - a.k.a. shouldn't be reached
    }

    assert(0); // This shouldn't be reached either
}

/*

Complexity: O(q) where q is the number of commands

Every operation except clear (and exit, which calls it internally) works in O(1) and is called no more than q times.
Clear performs a number of actions proportional to the stack's current size, so if we count every call to it,
for every element added clear will only go over it once, so in total all clear calls will remain linear.


P.S. - Sorry If I'm doing it wrong by writing comments in English, it just seems to be part of the proper code style to me.

*/

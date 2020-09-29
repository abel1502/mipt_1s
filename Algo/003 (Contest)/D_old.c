/*

Гоблины Мглистых гор очень любят ходить к своим шаманам. Так как гоблинов много, к шаманам часто образуются очень длинные очереди. А поскольку много гоблинов в одном месте быстро образуют шумную толку, которая мешает шаманам проводить сложные медицинские манипуляции, последние решили установить некоторые правила касательно порядка в очереди.

Обычные гоблины при посещении шаманов должны вставать в конец очереди. Привилегированные же гоблины, знающие особый пароль, встают ровно в ее середину, причем при нечетной длине очереди они встают сразу за центром.

Так как гоблины также широко известны своим непочтительным отношением ко всяческим правилам и законам, шаманы попросили вас написать программу, которая бы отслеживала порядок гоблинов в очереди.

Входные данные
В первой строке входных данный записано число N (1?N?105)  ? количество запросов. Следующие N строк содержат описание запросов в формате:

+ i  ? гоблин с номером i (1?i?N) встаёт в конец очереди.
* i  ? привилегированный гоблин с номером i встает в середину очереди.
-  ? первый гоблин из очереди уходит к шаманам. Гарантируется, что на момент такого запроса очередь не пуста.
Выходные данные
Для каждого запроса типа - программа должна вывести номер гоблина, который должен зайти к шаманам.

Примеры
входные данные
7
+ 1
+ 2
-
+ 3
+ 4
-
-
выходные данные
1
2
3
входные данные
2
* 1
+ 2
выходные данные

*/


#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>


typedef unsigned int item_t;

typedef enum {
    ERR_OK,  // Has to be equal to 0 for compact success checks
    ERR_NULL,
    ERR_OVERFLOW,
    ERR_UNDERFLOW
} err_code;


//--------------------------------------------------------------------------------
// Stack
//--------------------------------------------------------------------------------

static const size_t MAX_STACK = 100001;


typedef struct stack_item_s stack_item;
typedef struct stack_s stack;


struct stack_item_s {
    item_t val;
    stack_item *prev;
};

struct stack_s {
    stack_item *top;
    size_t size;
};


stack *stack_new();
err_code stack_delete(stack *self);
err_code stack_push(stack *self, item_t value);
err_code stack_pop(stack *self, item_t *value);
err_code stack_peek(stack *self, item_t *value);
err_code stack_clear(stack *self);


stack *stack_new() {
    stack *self = (stack *)malloc(sizeof(stack));
    if (self == NULL) {
        return NULL;
    }

    self->size = 0;
    self->top = NULL;

    return self;
}

err_code stack_delete(stack *self) {
    if (self == NULL) {
        return ERR_NULL;
    }

    stack_clear(self);  // We actually can't return here because a destructor must always work completely

    free(self);

    return ERR_OK;
}

err_code stack_push(stack *self, item_t value) {
    if (self == NULL) {
        return ERR_NULL;
    }

    if (self->size >= MAX_STACK) {
        return ERR_OVERFLOW;
    }

    stack_item *newTop = (stack_item *)malloc(sizeof(stack_item));
    if (newTop == NULL) {
        return ERR_OVERFLOW;
    }

    newTop->prev = self->top;
    newTop->val = value;

    self->top = newTop;
    assert(self->size < MAX_STACK);
    self->size++;

    return ERR_OK;
}

err_code stack_pop(stack *self, item_t *value) {
    if (self == NULL || value == NULL) {
        return ERR_NULL;
    }

    if (self->top == NULL) {
        return ERR_UNDERFLOW;
    }

    stack_item *oldTop = self->top;

    *value = oldTop->val;

    self->top = oldTop->prev;
    assert(self->size > 0);
    self->size--;
    free(oldTop);

    return ERR_OK;
}

err_code stack_peek(stack *self, item_t *value) {
    if (self == NULL || value == NULL) {
        return ERR_NULL;
    }

    if (self->top == NULL) {
        return ERR_UNDERFLOW;
    }

    *value = self->top->val;

    return ERR_OK;
}

err_code stack_clear(stack *self) {
    if (self == NULL) {
        return ERR_NULL;
    }

    item_t tmp;  // = 0; // Can't be initialized because item_t in general may be both a struct or a number, for example
    while (self->size > 0) {
        stack_pop(self, &tmp);
    }
    return ERR_OK;
}

//================================================================================

//--------------------------------------------------------------------------------
// Queue
//--------------------------------------------------------------------------------

typedef struct queue_s queue;


struct queue_s {
    stack *stkIn;
    stack *stkOut;
    size_t size;
};

queue *queue_new();
err_code queue_delete(queue *self);
err_code queue_push(queue *self, item_t value);
err_code queue_pop(queue *self, item_t *value);
err_code queue_peek(queue *self, item_t *value);
static void queue_relay(queue *self);
err_code queue_clear(queue *self);


queue *queue_new() {
    queue *self = (queue *)malloc(sizeof(queue));
    if (self == NULL) {
        return NULL;
    }

    self->size = 0;

    self->stkIn  = stack_new();
    self->stkOut = stack_new();
    if (self->stkIn == NULL || self->stkOut == NULL) {
        queue_delete(self);
        return NULL;
    }

    return self;
}

err_code queue_delete(queue *self) {
    if (self == NULL || self->stkOut == NULL || self->stkIn == NULL) {
        return ERR_NULL;
    }

    self->size = 0;

    stack_delete(self->stkIn);
    stack_delete(self->stkOut);

    free(self);

    return ERR_OK;
}

err_code queue_push(queue *self, item_t value) {
    if (self == NULL || self->stkOut == NULL || self->stkIn == NULL) {
        return ERR_NULL;
    }

    err_code res;
    if ((res = stack_push(self->stkIn, value)) != ERR_OK) {
        return res;
    }

    self->size++;

    return ERR_OK;
}

err_code queue_pop(queue *self, item_t *value) {
    if (self == NULL || self->stkOut == NULL || self->stkIn == NULL) {
        return ERR_NULL;
    }

    if (self->stkOut->size == 0) {
        queue_relay(self);
    }

    err_code res = stack_pop(self->stkOut, value);
    if (res != ERR_OK) {
        return res;
    }

    self->size--;

    return ERR_OK;
}

err_code queue_peek(queue *self, item_t *value) {
    if (self == NULL || self->stkOut == NULL || self->stkIn == NULL) {
        return ERR_NULL;
    }

    if (self->stkOut->size == 0) {
        queue_relay(self);
    }

    err_code res = stack_peek(self->stkOut, value);
    if (res != ERR_OK) {
        return res;
    }

    return ERR_OK;
}

static void queue_relay(queue *self) {
    assert(self != NULL);
    assert(self->stkIn != NULL);
    assert(self->stkOut != NULL);
    assert(self->stkOut->size == 0);

    while (self->stkIn->size != 0) {
        item_t value;
        err_code res;

        res = stack_pop(self->stkIn, &value);
        assert(res == ERR_OK);

        res = stack_push(self->stkOut, value);
        assert(res == ERR_OK);
    }
}

err_code queue_clear(queue *self) {
    if (self == NULL || self->stkOut == NULL || self->stkIn == NULL) {
        return ERR_NULL;
    }

    err_code res;

    if ((res = stack_clear(self->stkIn)) != ERR_OK) {
        return res;
    }

    if ((res = stack_clear(self->stkOut)) != ERR_OK) {
        return res;
    }

    self->size = 0;

    return ERR_OK;
}

//================================================================================

int main() {
    item_t n = 0;
    scanf("%u", n);

    queue *queueFront = queue_new();
    queue *queueMid   = queue_new();
    queue *queueBack  = queue_new();

    for (item_t i = 0; i < n; ++i) {
        char cmd = '\0';
        item_t arg = 0;

        scanf('%c', &cmd);

        switch (cmd) {
        case '+':
            scanf("%u", &arg);
            queue_push(queueBack, arg);

            if (queue_size(secondHalf) > queue_size(firstHalf) + 1) {
                queue_pop(secondHalf, &arg);
                queue_push(firstHalf, arg);
            }

            break;
        case '*':
            scanf("%u", &arg);
            queue_push(firstHalf, arg);
            break;
        case '-':
            assert(queue_size(firstHalf) > 0);
            queue_pop(firstHalf, &arg);
            printf("%u", arg);
            break;
        default:
            assert(0);
        }
    }

    return 0;
}

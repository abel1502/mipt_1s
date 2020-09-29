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

static const size_t MAX_DEQUE = 100001;

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

//--------------------------------------------------------------------------------

int main() {
    item_t n = 0;
    scanf("%u", &n);

    deque *front = deque_new();
    deque *back  = deque_new();

    for (item_t i = 0; i < n; ++i) {
        //deque_dump(back);
        //deque_dump(front);

        char cmd = '\0';
        item_t arg = 0;

        scanf(" %c", &cmd);

        switch (cmd) {
        case '+':
            scanf("%u", &arg);
            deque_pushBack(back, arg);

            if (back->size > front->size) {
                deque_pushBack(front, deque_popFront(back));
            }

            break;
        case '*':
            scanf("%u", &arg);
            deque_pushBack(front, arg);

            if (front->size > back->size + 1) {
                deque_pushFront(back, deque_popBack(front));
            }

            break;
        case '-':
            assert(front->size > 0);
            //printf("> %zu %zu\n", back->size, front->size);
            printf("%u\n", deque_popFront(front));

            if (back->size > front->size) {
                deque_pushBack(front, deque_popFront(back));
            }

            break;
        default:
            assert(0);
        }
    }

    deque_delete(front);
    deque_delete(back);

    return 0;
}

/*

Complexity: Linear because dequeue operations are O(1) and we perform up to a constant amount of them per command

*/

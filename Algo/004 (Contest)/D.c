/*

–еализуйте двоичную кучу.

ќбработайте запросы следующих видов:

insert x: вставить целое число x в кучу;
getMin: вывести значение минимального элемента в куче (гарантируетс€, что к этому моменту куча не пуста);
extractMin: удалить минимальный элемент из кучи (гарантируетс€, что к этому моменту куча не пуста);
decreaseKey i \N{GREEK CAPITAL LETTER DELTA}: уменьшить число, вставленное на i-м запросе, на целое число \N{GREEK CAPITAL LETTER DELTA}\N{GREATER-THAN OR EQUAL TO}0 (гарантируетс€, что i-й запрос был осуществлЄн ранее, €вл€лс€ запросом добавлени€, а добавленное на этом шаге число всЄ ещЄ лежит в куче).
ћожете считать, что в любой момент времени все числа, хран€щиес€ в куче, попарно различны, а их количество не превышает 105.

¬ходные данные
¬ первой строке содержитс€ число q (1\N{LESS-THAN OR EQUAL TO}q\N{LESS-THAN OR EQUAL TO}106), означающее число запросов.

¬ следующих q строках содержатс€ запросы в описанном выше формате. ƒобавл€емые числа x и поправки \N{GREEK CAPITAL LETTER DELTA} лежат в промежутке [\N{MINUS SIGN}109,109], а \N{GREEK CAPITAL LETTER DELTA}\N{GREATER-THAN OR EQUAL TO}0.

¬ыходные данные
Ќа каждый запрос вида getMin выведите ответ в отдельной строке.

ѕримеры
входные данные
5
insert 5
decreaseKey 1 3
getMin
insert 1
getMin
выходные данные
2
1
входные данные
5
insert 5
getMin
extractMin
insert 10
getMin
выходные данные
5
10

*/

#ifdef ALGO_LOCAL
#define __USE_MINGW_ANSI_STDIO 1
#endif

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

//--------------------------------------------------------------------------------

typedef int heap_item_t;

typedef struct heap_s heap_t;


struct heap_s {
    size_t size;
    size_t capacity;
    heap_item_t data[1];
};

//--------------------------------------------------------------------------------

// TODO: ? Replace size_t with unsigned
static const size_t MAX_CMD_LEN = 12;  // strlen("decreaseKey") + 1;
static const char CMD_FMT[] = "%11s";

//--------------------------------------------------------------------------------

heap_t *heap_new(size_t capacity);

void heap_free(heap_t *self);

void heap_push(heap_t *self, heap_item_t value);

void heap_top(heap_t *self, heap_item_t *value);

void heap_pop(heap_t *self, heap_item_t *value);

void heap_decreaseKey(heap_t *self, )

static void heap_swap(heap_t *self, size_t a, size_t b);

static void heap_siftDown(heap_t *self, size_t ind);

static void heap_siftUp(heap_t *self, size_t ind);

void heap_dump(heap_t *self);

//--------------------------------------------------------------------------------


int main() {
    int res = 0;

    unsigned q = 0;
    res = scanf("%u", &q);
    assert(res == 1);

    heap_t *heap = heap_new(q);

    size_t *elemLookup = (size_t *)calloc(q, sizeof(heap_item_t));

    for (unsigned i = 0; i < q; ++i) {
        char cmd[MAX_CMD_LEN];

        res = scanf(CMD_FMT, cmd);
        assert(res == 1);

        if (strcmp(cmd, "insert") == 0) {
            heap_item_t arg = 0;

            res = scanf("%i", &arg);
            assert(res == 1);

            heap_push(heap, arg);
        } else if (strcmp(cmd, "getMin") == 0) {
            heap_item_t arg = 0;

            heap_top(heap, &arg);

            printf("%i\n", arg);
        } else if (strcmp(cmd, "extractMin") == 0) {
            heap_pop(heap, NULL);
        } else if (strcmp(cmd, "decreaseKey") == 0) {
            res = scanf("%i", &arg);
            assert(res == 1);
        #ifdef ALGO_LOCAL
        } else if (strcmp(cmd, "dump") == 0) {
            heap_dump(heap);
        #endif // ALGO_LOCAL
        } else {
            assert(0);
        }
    }

    free(elemLookup);

    heap_free(heap);

    return 0;
}

//--------------------------------------------------------------------------------

heap_t *heap_new(size_t capacity) {
    heap_t *self = (heap_t *)calloc(1, sizeof(heap_t) + capacity * sizeof(heap_item_t));
    assert(self != NULL);

    self->capacity = capacity;
    self->size = 0;

    return self;
}

void heap_free(heap_t *self) {
    free(self);
}

void heap_push(heap_t *self, heap_item_t value) {
    assert(self != NULL);
    assert(self->size + 1 <= self->capacity);

    self->size++;

    self->data[self->size] = value;

    heap_siftUp(self, self->size);
}

void heap_top(heap_t *self, heap_item_t *value) {
    assert(self != NULL);
    assert(value != NULL);

    assert(self->size > 0);

    *value = self->data[1];
}

void heap_pop(heap_t *self, heap_item_t *value) {
    assert(self != NULL);

    assert(self->size > 0);

    if (value != NULL) {
        *value = self->data[1];
    }

    heap_swap(self, 1, self->size);

    self->size--;

    if (self->size > 0)
        heap_siftDown(self, 1);
}

static void heap_swap(heap_t *self, size_t a, size_t b) {
    assert(self != NULL);
    assert(1 <= a && a <= self->size);
    assert(1 <= b && b <= self->size);

    heap_item_t tmp = self->data[a];
    self->data[a] = self->data[b];
    self->data[b] = tmp;
}

static void heap_siftDown(heap_t *self, size_t ind) {
    assert(self != NULL);
    assert(1 <= ind && ind <= self->size);

    while (2 * ind <= self->size) {
        size_t minInd = ind;
        heap_item_t min3 = self->data[ind];

        if (min3 > self->data[2 * ind]) {
            min3 = self->data[2 * ind];
            minInd = 2 * ind;
        }

        if (2 * ind + 1 <= self->size && min3 > self->data[2 * ind + 1]) {
            min3 = self->data[2 * ind + 1];
            minInd = 2 * ind + 1;
        }

        if (minInd == ind) break;

        heap_swap(self, ind, minInd);

        ind = minInd;
    }
}

static void heap_siftUp(heap_t *self, size_t ind) {
    assert(self != NULL);
    assert(1 <= ind && ind <= self->size);

    while (ind > 1 && self->data[ind] < self->data[ind / 2]) {
        heap_swap(self, ind, ind / 2);
        ind /= 2;
    }
}

void heap_dump(heap_t *self) {
    printf("heap_t [0x%p] {\n", self);
    if (self != NULL) {
        printf("  size     = %zu\n", self->size);
        printf("  capacity = %zu\n", self->capacity);

        printf("  data [0x%p] {\n", self->data);
        if (self->data != NULL) {
            for (size_t i = 0; i <= self->capacity; ++i) {
                char lineMarker = 0;

                if (i > 0 && i <= self->size) {
                    lineMarker = '*';
                } else {
                    lineMarker = ' ';
                }

                printf("  %c [%2zu] = ", lineMarker, i);

                printf("0x");
                for (size_t j = 0; j < sizeof(heap_item_t); ++j) {
                    printf("%02X", ((unsigned char *)(self->data + i))[j]);
                }
                printf(" ");

                printf("(");
                printf("%i", self->data[i]);
                printf(")");

                printf("\n");
            }
        } else {
            printf("    <corrupt>\n");
        }

        printf("  }\n");
    } else {
        printf("  <corrupt>\n");
    }

    printf("}\n\n");

    fflush(stdout);
    fflush(stderr);
}


/*

Complexity: O(q log q), since it's a heap

*/

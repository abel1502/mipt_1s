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


//#define assert0(stmt) if (!(stmt)) { printf("qqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqq\n"); exit(0); }
#define assert0 assert

//--------------------------------------------------------------------------------

// I had to replace size_t with unsigned everywhere for this to work on CF - sorry


typedef struct {
    long long int value;
    long long unsigned index;  // The number of the request this item was added on
} heap_item_t;

typedef struct heap_s heap_t;


struct heap_s {
    long long unsigned size;
    long long unsigned capacity;
    long long unsigned *indices;  // A lookup table, where indices[i] == j means that the item added on the i'th request is currently in data[j]
    heap_item_t data[1];
};

//--------------------------------------------------------------------------------

static const long long unsigned MAX_CMD_LEN = 12;  // strlen("decreaseKey") + 1;
static const char CMD_FMT[] = "%11s";

//--------------------------------------------------------------------------------

heap_t *heap_new(long long unsigned capacity);

void heap_free(heap_t *self);

void heap_push(heap_t *self, heap_item_t value);

void heap_top(heap_t *self, heap_item_t *value);

void heap_pop(heap_t *self, heap_item_t *value);

void heap_decreaseKey(heap_t *self, long long unsigned ind, long long int delta);

static void heap_swap(heap_t *self, long long unsigned a, long long unsigned b);

static void heap_siftDown(heap_t *self, long long unsigned ind);

static void heap_siftUp(heap_t *self, long long unsigned ind);

void heap_dump(heap_t *self);

//--------------------------------------------------------------------------------


int main() {
    int res = 0;

    long long unsigned q = 0;
    res = scanf("%llu", &q);
    assert(res == 1);

    heap_t *heap = heap_new(q);

    for (long long unsigned i = 0; i < q; ++i) {
        char cmd[MAX_CMD_LEN];

        res = scanf(CMD_FMT, cmd);
        assert(res == 1);

        if (strcmp(cmd, "insert") == 0) {
            heap_item_t arg = {0, i};

            res = scanf("%lld", &arg.value);
            assert(res == 1);

            heap_push(heap, arg);
        } else if (strcmp(cmd, "getMin") == 0) {
            heap_item_t arg = {};

            heap_top(heap, &arg);

            printf("%lld\n", arg.value);
        } else if (strcmp(cmd, "extractMin") == 0) {
            heap_pop(heap, NULL);
        } else if (strcmp(cmd, "decreaseKey") == 0) {
            long long unsigned ind = 0;
            long long int delta = 0;
            res = scanf("%llu %lld", &ind, &delta);
            assert(res == 2);

            heap_decreaseKey(heap, ind - 1, delta);
        #ifdef ALGO_LOCAL
        } else if (strcmp(cmd, "dump") == 0) {
            heap_dump(heap);
        #endif // ALGO_LOCAL
        } else {
            assert(0);
        }
    }

    heap_free(heap);

    return 0;
}

//--------------------------------------------------------------------------------

// Hopefully I don't have to explain how and why the heap works)

heap_t *heap_new(long long unsigned capacity) {
    heap_t *self = (heap_t *)calloc(1, sizeof(heap_t) + capacity * sizeof(heap_item_t));
    assert(self != NULL);

    self->capacity = capacity;
    self->size = 0;

    self->indices = (long long unsigned *)calloc(capacity + 1, sizeof(long long unsigned));  // We can have it the same size, because the capacity we initialize
                                                                     // our heap with is equal to the number of requests
    assert(self->indices != NULL);

    return self;
}

void heap_free(heap_t *self) {
    assert(self != NULL);

    free(self->indices);

    free(self);
}

void heap_push(heap_t *self, heap_item_t value) {
    assert(self != NULL);
    assert0(self->size + 1 <= self->capacity);  // CHECK

    self->size++;

    self->data[self->size] = value;

    self->indices[value.index] = self->size;

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

    //self->indices[self->data[self->size].index] = -1;

    self->size--;

    //if (self->size > 0)
    heap_siftDown(self, 1);
}

void heap_decreaseKey(heap_t *self, long long unsigned ind, long long int delta) {
    assert(self != NULL);
    assert(self->indices != NULL);
    assert0(ind < self->capacity);  // CHECK

    assert0(self->indices[ind] != 0);

    //assert(self->indices[ind] >= 1);  //!
    //if (self->indices[ind] == self->size + 1) {
    //    assert(0);
    //}
    //assert(self->indices[ind] <= self->size);  //!!!

    self->data[self->indices[ind]].value -= delta;

    heap_siftUp(self, self->indices[ind]);  //!!
}

static void heap_swap(heap_t *self, long long unsigned a, long long unsigned b) {
    assert(self != NULL);
    assert(self->indices != NULL);
    //assert0(1 <= a && 1 <= b);
    assert0(1 <= a && a </*=*/ self->capacity /*size*/);  // CHECK
    assert0(1 <= b && b </*=*/ self->capacity /*size*/);  // CHECK

    //printf(">> (was) %llu = %llu, %llu = %llu\n", self->data[a].index, self->indices[self->data[a].index], self->data[b].index, self->indices[self->data[b].index]);

    /*long long unsigned tmp = self->indices[self->data[a].index];
    self->indices[self->data[a].index] = self->indices[self->data[b].index];
    self->indices[self->data[b].index] = tmp;*/

    heap_item_t tmp2 = self->data[a];
    self->data[a] = self->data[b];
    self->data[b] = tmp2;

    self->indices[self->data[a].index] = a;
    self->indices[self->data[b].index] = b;

    //printf(">> ( is) %llu = %llu, %llu = %llu\n", self->data[a].index, self->indices[self->data[a].index], self->data[b].index, self->indices[self->data[b].index]);
}

static void heap_siftDown(heap_t *self, long long unsigned ind) {
    assert(self != NULL);
    assert(self->indices != NULL);
    //assert0(1 <= ind);
    assert0(1 <= ind /*&& ind <= self->size*/);  // CHECK

    while (2 * ind <= self->size) {
        long long unsigned minInd = ind;
        int min3 = self->data[ind].value;

        if (min3 > self->data[2 * ind].value) {
            min3 = self->data[2 * ind].value;
            minInd = 2 * ind;
        }

        if (2 * ind + 1 <= self->size && min3 > self->data[2 * ind + 1].value) {
            min3 = self->data[2 * ind + 1].value;
            minInd = 2 * ind + 1;
        }

        if (minInd == ind) break;

        heap_swap(self, ind, minInd);

        ind = minInd;
    }
}

static void heap_siftUp(heap_t *self, long long unsigned ind) {
    assert(self != NULL);
    assert(self->indices != NULL);
    //assert0(1 <= ind);
    assert0(/*1 <= ind &&*/ ind </*=*/ self->capacity /*size*/);  // CHECK

    while (ind > 1 && self->data[ind].value < self->data[ind / 2].value) {
        heap_swap(self, ind, ind / 2);
        ind /= 2;
    }
}

void heap_dump(heap_t *self) {
    printf("heap_t [0x%p] {\n", self);
    if (self != NULL) {
        printf("  size     = %llu\n", self->size);
        printf("  capacity = %llu\n", self->capacity);

        printf("  data [0x%p] {\n", self->data);
        if (self->data != NULL) {
            for (long long unsigned i = 0; i <= self->capacity; ++i) {
                char lineMarker = 0;

                if (i > 0 && i <= self->size) {
                    lineMarker = '*';
                } else {
                    lineMarker = ' ';
                }

                printf("  %c [%2llu] = ", lineMarker, i);

                printf("0x");
                for (long long unsigned j = 0; j < sizeof(heap_item_t); ++j) {
                    printf("%02X", ((unsigned char *)(self->data + i))[j]);
                }
                printf(" ");

                printf("(");
                printf("%lld at %llu", self->data[i].value, self->data[i].index);
                printf(")");

                printf("\n");
            }
        } else {
            printf("    <corrupt>\n");
        }

        printf("  }\n");

        printf("  indices [0x%p] {\n", self->indices);
        if (self->indices != NULL) {
            for (long long unsigned i = 0; i <= self->capacity; ++i) {
                printf("    [%2llu] = %llu\n", i, self->indices[i]);
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

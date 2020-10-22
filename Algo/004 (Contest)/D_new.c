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

//--------------------------------------------------------------------------------

// I had to replace size_t with unsigned everywhere for this to work on CF - sorry


typedef long long int heap_item_t;

typedef struct heap_s heap_t;


struct heap_s {
    unsigned size;
    unsigned capacity;
    unsigned *lookupByRequest;  // <this field>[i] == j means that the number inserted in the i'th request is in the j'th cell
    unsigned *lookupByValue;    // <this field>[i] == j means that the i'th cell contains the number inserted in the j'th request
    heap_item_t data[1];
};

//--------------------------------------------------------------------------------

static const unsigned MAX_CMD_LEN = 16;  // >= strlen("decreaseKey") + 1;
static const char CMD_FMT[] = "%15s";

//--------------------------------------------------------------------------------

heap_t *heap_new(unsigned capacity);

void heap_free(heap_t *self);

void heap_push(heap_t *self, heap_item_t value, unsigned reqInd);

void heap_top(heap_t *self, heap_item_t *value);

void heap_pop(heap_t *self, heap_item_t *value);

void heap_decreaseKey(heap_t *self, unsigned reqInd, heap_item_t delta);

static void heap_swap(heap_t *self, unsigned a, unsigned b);

static void heap_siftDown(heap_t *self, unsigned ind);

static void heap_siftUp(heap_t *self, unsigned ind);

void heap_dump(heap_t *self);

//--------------------------------------------------------------------------------


int main() {
    int res = 0;

    unsigned q = 0;
    res = scanf("%u", &q);
    assert(res == 1);

    heap_t *heap = heap_new(q + 1);

    for (unsigned i = 0; i < q; ++i) {
        char cmd[MAX_CMD_LEN];

        res = scanf(CMD_FMT, cmd);
        assert(res == 1);

        if (strcmp(cmd, "insert") == 0) {
            heap_item_t arg = 0;

            res = scanf("%lld", &arg);
            assert(res == 1);

            heap_push(heap, arg, i);
        } else if (strcmp(cmd, "getMin") == 0) {
            heap_item_t arg = 0;

            heap_top(heap, &arg);

            printf("%lld\n", arg);
        } else if (strcmp(cmd, "extractMin") == 0) {
            heap_pop(heap, NULL);
        } else if (strcmp(cmd, "decreaseKey") == 0) {
            unsigned ind = 0;
            long long int delta = 0;
            res = scanf("%u %lld", &ind, &delta);
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

heap_t *heap_new(unsigned capacity) {
    heap_t *self = (heap_t *)calloc(1, sizeof(heap_t) + capacity * sizeof(heap_item_t));
    assert(self != NULL);

    self->capacity = capacity;
    self->size = 1;

    self->lookupByRequest = (unsigned *)calloc(capacity, sizeof(self->lookupByRequest[0]));
    assert(self->lookupByRequest != NULL);

    self->lookupByValue = (unsigned *)calloc(capacity, sizeof(self->lookupByValue[0]));
    assert(self->lookupByValue != NULL);

    return self;
}

void heap_free(heap_t *self) {
    assert(self != NULL);

    free(self->lookupByRequest);

    free(self->lookupByValue);

    free(self);
}

void heap_push(heap_t *self, heap_item_t value, unsigned reqInd) {
    assert(self != NULL);
    assert(self->size <= self->capacity);

    self->data[self->size] = value;

    self->lookupByRequest[reqInd] = self->size;

    self->lookupByValue[self->size] = reqInd;

    self->size++;

    heap_siftUp(self, self->size - 1);


}

void heap_top(heap_t *self, heap_item_t *value) {
    assert(self != NULL);
    assert(value != NULL);

    assert(self->size > 1);

    *value = self->data[1];
}

void heap_pop(heap_t *self, heap_item_t *value) {
    assert(self != NULL);

    assert(self->size > 1);

    if (value != NULL) {
        *value = self->data[1];
    }

    --self->size;

    heap_swap(self, 1, self->size);

    heap_siftDown(self, 1);
}

void heap_decreaseKey(heap_t *self, unsigned reqInd, heap_item_t delta) {
    assert(self != NULL);
    assert(reqInd < self->capacity);

    self->data[self->lookupByRequest[reqInd]] -= delta;

    heap_siftUp(self, self->lookupByRequest[reqInd]);
}

static void heap_swap(heap_t *self, unsigned a, unsigned b) {
    assert(self != NULL);
    assert(1 <= a && a < self->capacity);
    assert(1 <= b && b < self->capacity);

    heap_item_t tmp = self->data[a];
    self->data[a] = self->data[b];
    self->data[b] = tmp;

    unsigned a_req = self->lookupByValue[a];
    unsigned b_req = self->lookupByValue[b];

    self->lookupByRequest[a_req] = b;
    self->lookupByRequest[b_req] = a;

    self->lookupByValue[a] = b_req;
    self->lookupByValue[b] = a_req;


}

static void heap_siftDown(heap_t *self, unsigned ind) {
    assert(self != NULL);
    assert(1 <= ind && ind < self->capacity);

    while (2 * ind < self->size) {
        unsigned left = 2 * ind;
        unsigned right = 2 * ind + 1;

        unsigned minInd = ind;
        long long int min3 = self->data[ind];

        if (min3 > self->data[left]) {
            min3 = self->data[left];
            minInd = left;
        }

        if (right < self->size && min3 > self->data[right]) {
            min3 = self->data[right];
            minInd = right;
        }

        if (minInd == ind) break;

        heap_swap(self, ind, minInd);

        ind = minInd;
    }
}

static void heap_siftUp(heap_t *self, unsigned ind) {
    assert(self != NULL);
    assert(1 <= ind && ind < self->capacity);

    if (ind >= self->size) return;

    while (ind > 1 && self->data[ind] < self->data[ind / 2]) {
        heap_swap(self, ind, ind / 2);
        ind /= 2;
    }
}

void heap_dump(heap_t *self) {
    printf("heap_t [0x%p] {\n", self);
    if (self != NULL) {
        printf("  size     = %u\n", self->size);
        printf("  capacity = %u\n", self->capacity);

        printf("  data [0x%p] {\n", self->data);
        if (self->data != NULL) {
            for (unsigned i = 0; i <= self->capacity; ++i) {
                char lineMarker = 0;

                if (i > 0 && i < self->size) {
                    lineMarker = '*';
                } else {
                    lineMarker = ' ';
                }

                printf("  %c [%2u] = %lld\n", lineMarker, i, self->data[i]);
            }
        } else {
            printf("    <corrupt>\n");
        }
        printf("  }\n");

        printf("  byRequest [0x%p] {\n", self->lookupByRequest);
        if (self->lookupByRequest != NULL) {
            for (unsigned i = 0; i < self->capacity; ++i) {
                printf("    [%2u] = %u\n", i, self->lookupByRequest[i]);
            }
        } else {
            printf("    <corrupt>\n");
        }
        printf("  }\n");

        printf("  byValue [0x%p] {\n", self->lookupByValue);
        if (self->lookupByValue != NULL) {
            for (unsigned i = 0; i < self->capacity; ++i) {
                printf("    [%2u] = %u\n", i, self->lookupByValue[i]);
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


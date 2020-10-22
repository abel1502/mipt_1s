/*

Дана очень длинная последовательность целых чисел длины N. Требуется вывести в отсортированном виде её наименьшие K элементов. Последовательность может не помещаться в память. Время работы O(N\N{DOT OPERATOR}logK), память O(K). 1\N{LESS-THAN OR EQUAL TO}N\N{LESS-THAN OR EQUAL TO}105, 1\N{LESS-THAN OR EQUAL TO}K\N{LESS-THAN OR EQUAL TO}500
Входные данные
В первой строке записаны N и K.

В следующей строке через пробел записана последовательность целых чисел.

Выходные данные
K наименьших элементов последовательности в отсортированном виде.

Пример
входные данные
9 4
3 7 4 5 6 1 15 4 2
выходные данные
1 2 3 4

*/

#ifdef ALGO_LOCAL
#define __USE_MINGW_ANSI_STDIO 1
#endif

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>


//--------------------------------------------------------------------------------


typedef long long int heap_item_t;

typedef struct heap_s heap_t;


struct heap_s {
    unsigned size;
    unsigned capacity;
    heap_item_t data[1];
};

//--------------------------------------------------------------------------------

heap_t *heap_new(unsigned capacity);

void heap_free(heap_t *self);

void heap_push(heap_t *self, heap_item_t value);

void heap_top(heap_t *self, heap_item_t *value);

void heap_pop(heap_t *self, heap_item_t *value);

static void heap_swap(heap_t *self, unsigned a, unsigned b);

static void heap_siftDown(heap_t *self, unsigned ind);

static void heap_siftUp(heap_t *self, unsigned ind);

void heap_dump(heap_t *self);

//--------------------------------------------------------------------------------


/*
  Essentially, we keep track of the k smallest items so far by maintaining a max-heap of them,
  and after adding a new element to it, removing the biggest one.
*/
int main() {
    int res = 0;

    unsigned n = 0, k = 0;
    res = scanf("%u %u", &n, &k);
    assert(res == 2);

    assert(n >= k);

    heap_t *heap = heap_new(k + 2);

    unsigned i = 0;

    for (; i < k; ++i) {
        heap_item_t cur = 0;

        res = scanf("%lld", &cur);
        assert(res == 1);

        heap_push(heap, cur);
    }

    for (; i < n; ++i) {
        heap_item_t cur = 0;

        res = scanf("%lld", &cur);
        assert(res == 1);

        heap_push(heap, cur);

        heap_pop(heap, NULL);
    }

    heap_item_t *result = (heap_item_t *)calloc(k, sizeof(result[0]));

    for (i = k; i > 0; --i) {
        heap_pop(heap, &result[i - 1]);
    }

    for (i = 0; i < k; ++i) {
        printf("%lld ", result[i]);
    }
    printf("\n");

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

    return self;
}

void heap_free(heap_t *self) {
    free(self);
}

void heap_push(heap_t *self, heap_item_t value) {
    assert(self != NULL);
    assert(self->size <= self->capacity);

    self->data[self->size] = value;

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

static void heap_swap(heap_t *self, unsigned a, unsigned b) {
    assert(self != NULL);
    assert(1 <= a && a < self->capacity);
    assert(1 <= b && b < self->capacity);

    heap_item_t tmp = self->data[a];
    self->data[a] = self->data[b];
    self->data[b] = tmp;


}

static void heap_siftDown(heap_t *self, unsigned ind) {
    assert(self != NULL);
    assert(1 <= ind && ind < self->capacity);

    while (2 * ind < self->size) {
        unsigned left = 2 * ind;
        unsigned right = 2 * ind + 1;

        unsigned minInd = ind;
        long long int min3 = self->data[ind];

        if (min3 < self->data[left]) {
            min3 = self->data[left];
            minInd = left;
        }

        if (right < self->size && min3 < self->data[right]) {
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

    while (ind > 1 && self->data[ind] > self->data[ind / 2]) {
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
    } else {
        printf("  <corrupt>\n");
    }

    printf("}\n\n");

    fflush(stdout);
    fflush(stderr);
}


/*

Complexity: O(n log k) memory, because we maintain a heap with O(k) items and perform O(n) operations on it;
            O(k) memory, because we only use an O(k) heap and one O(k) array for result.

*/



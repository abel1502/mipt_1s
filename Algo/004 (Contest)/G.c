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

typedef long long heap_item_t;

typedef struct heap_s heap_t;

typedef struct biheap_s biheap_t;


struct heap_s {
    unsigned size;
    unsigned capacity;
    int coeff;
    unsigned *lookupByRequest;  // <this field>[i] == j means that the number inserted in the i'th request is in the j'th cell
    unsigned *lookupByValue;    // <this field>[i] == j means that the i'th cell contains the number inserted in the j'th request
    heap_item_t data[1];
};

struct biheap_s {
    unsigned size;
    unsigned capacity;
    heap_t *minHeap;
    heap_t *maxHeap;
};

//--------------------------------------------------------------------------------

static const unsigned CMD_LEN = 7;  // All commands happen to share the same length, so I'll use it)
static const char CMD_FMT[] = "%6s";

static const heap_item_t MAX_NUM = 0xffffffff0000;

//--------------------------------------------------------------------------------

heap_t *heap_new(unsigned capacity, int isMin);

void heap_free(heap_t *self);

void heap_push(heap_t *self, heap_item_t value, unsigned reqInd);

void heap_top(heap_t *self, heap_item_t *value);

void heap_pop(heap_t *self, heap_item_t *value);

void heap_decreaseKey(heap_t *self, unsigned reqInd, heap_item_t delta);

static void heap_swap(heap_t *self, unsigned a, unsigned b);

static void heap_siftDown(heap_t *self, unsigned ind);

static void heap_siftUp(heap_t *self, unsigned ind);

void heap_dump(heap_t *self);


biheap_t *biheap_new(unsigned capacity);

void biheap_free(biheap_t *self);

void biheap_push(biheap_t *self, heap_item_t value);

void biheap_popMin(biheap_t *self, heap_item_t *value);

void biheap_popMax(biheap_t *self, heap_item_t *value);

void biheap_dump(biheap_t *self);

//--------------------------------------------------------------------------------


int main() {
    int res = 0;

    unsigned n = 0;

    res = scanf("%u", &n);
    assert(res == 1);

    biheap_t *biheap = biheap_new(n + 1);

    for (unsigned i = 0; i < n; ++i) {
        char cmd[CMD_LEN];

        res = scanf(CMD_FMT, cmd);
        assert(res == 1);

        if (strcmp(cmd, "Insert") == 0) {
            heap_item_t arg = 0;

            res = scanf("(%lld)", &arg);
            assert(res == 1);

            biheap_push(biheap, arg);
        } else if (strcmp(cmd, "GetMin") == 0) {
            heap_item_t arg = 0;

            biheap_popMin(biheap, &arg);

            printf("%lld\n", arg);
        } else if (strcmp(cmd, "GetMax") == 0) {
            heap_item_t arg = 0;

            biheap_popMax(biheap, &arg);

            printf("%lld\n", arg);
        #ifdef ALGO_LOCAL
        } else if (strcmp(cmd, "dump") == 0) {
            biheap_dump(biheap);
            i--;
        #endif // ALGO_LOCAL
        } else {
            assert(0);
        }
        //biheap_dump(biheap);
    }

    biheap_free(biheap);

    return 0;
}

//--------------------------------------------------------------------------------

/*
  Heap is the same as in D, except now we have a coefficient to have the same
  implementation for both min- and max-heap.
*/

heap_t *heap_new(unsigned capacity, int isMin) {
    heap_t *self = (heap_t *)calloc(1, sizeof(heap_t) + capacity * sizeof(heap_item_t));
    assert(self != NULL);

    self->capacity = capacity;
    self->size = 1;

    self->coeff = isMin ? 1 : -1;

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

    self->data[self->size] = value * self->coeff;

    self->lookupByRequest[reqInd] = self->size;

    self->lookupByValue[self->size] = reqInd;

    self->size++;

    heap_siftUp(self, self->size - 1);


}

void heap_top(heap_t *self, heap_item_t *value) {
    assert(self != NULL);
    assert(value != NULL);

    assert(self->size > 1);

    *value = self->data[1] * self->coeff;
}

void heap_pop(heap_t *self, heap_item_t *value) {
    assert(self != NULL);

    assert(self->size > 1);

    if (value != NULL) {
        *value = self->data[1] * self->coeff;
    }

    --self->size;

    heap_swap(self, 1, self->size);

    heap_siftDown(self, 1);
}

void heap_decreaseKey(heap_t *self, unsigned reqInd, heap_item_t delta) {
    assert(self != NULL);
    assert(reqInd < self->capacity);
    assert(delta >= 0);

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
        printf("  coeff    = %d\n", self->coeff);

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

        printf("  byRequest [0x%p] {\n    ", self->lookupByRequest);
        if (self->lookupByRequest != NULL) {
            for (unsigned i = 0; i < self->capacity; ++i) {
                printf("%u, ", self->lookupByRequest[i]);
            }
        } else {
            printf("    <corrupt>\n");
        }
        printf("\n  }\n");

        printf("  byValue [0x%p] {\n    ", self->lookupByValue);
        if (self->lookupByValue != NULL) {
            for (unsigned i = 0; i < self->capacity; ++i) {
                printf("%u, ", self->lookupByValue[i]);
            }
        } else {
            printf("    <corrupt>\n");
        }
        printf("\n  }\n");
    } else {
        printf("  <corrupt>\n");
    }

    printf("}\n\n");

    fflush(stdout);
    fflush(stderr);
}


/*
  Here I could have implemented the same data structure as in the first theoretical problem, but a friend advised me that there's
  a simpler approach, so I guess I've found it. Essentially, we may use decreaseKey to pop the minimal element from the
  max heap, and vice-versa (and to identify, wht key to decrease, we use the same external indexing)
*/

biheap_t *biheap_new(unsigned capacity) {
    biheap_t *self = (biheap_t *)calloc(1, sizeof(*self));
    assert(self != NULL);

    self->maxHeap = heap_new(capacity, 0);
    self->minHeap = heap_new(capacity, 1);

    self->capacity = capacity;
    self->size = 0;

    return self;
}

void biheap_free(biheap_t *self) {
    assert(self != NULL);

    heap_free(self->minHeap);
    heap_free(self->maxHeap);

    free(self);
}

void biheap_push(biheap_t *self, heap_item_t value) {
    assert(self != NULL);
    assert(self->size < self->capacity);

    heap_push(self->minHeap, value, self->size);
    heap_push(self->maxHeap, value, self->size);

    self->size++;
}

void biheap_popMin(biheap_t *self, heap_item_t *value) {
    assert(self != NULL);

    unsigned target = self->minHeap->lookupByValue[1];

    //printf(">> %lld == %lld\n", self->minHeap->data[1], -self->maxHeap->data[self->maxHeap->lookupByRequest[target]]);

    heap_top(self->minHeap, value);

    //printf("pre: ");
    //heap_dump(self->maxHeap);

    /*heap_dump(self->minHeap);

    for (unsigned i = 0; i < self->minHeap->capacity; ++i) {
        printf("(%u, %u, %lld) ", self->minHeap->lookupByValue[i], self->minHeap->lookupByRequest[self->minHeap->lookupByValue[i]], self->minHeap->data[self->minHeap->lookupByRequest[self->minHeap->lookupByValue[i]]]);
    }
    printf("\n");

    printf("target: %u (%lld %lld)\n", target, self->minHeap->data[self->minHeap->lookupByRequest[target]], self->maxHeap->data[self->maxHeap->lookupByRequest[target]]);
    */

    heap_decreaseKey(self->maxHeap, target, MAX_NUM - *value);  // Despite the name, for a max heap this increases the value, and this specific delta causes it to become the biggest number in the heap...

    //printf("mid: ");
    //heap_dump(self->maxHeap);

    heap_pop(self->maxHeap, NULL);  // ... So now we can safely pop it

    //printf("post: ");
    //heap_dump(self->maxHeap);

    heap_pop(self->minHeap, NULL);
}

void biheap_popMax(biheap_t *self, heap_item_t *value) {
    assert(self != NULL);

    unsigned target = self->maxHeap->lookupByValue[1];

    //printf(">> %lld == %lld\n", -self->maxHeap->data[1], self->minHeap->data[self->minHeap->lookupByRequest[target]]);

    heap_top(self->maxHeap, value);

    //printf("pre: ");
    //heap_dump(self->minHeap);

    heap_decreaseKey(self->minHeap, target, MAX_NUM - *value);  // Same as above

    //printf("mid: ");
    //heap_dump(self->minHeap);

    heap_pop(self->minHeap, NULL);

    //printf("post: ");
    //heap_dump(self->minHeap);

    heap_pop(self->maxHeap, NULL);
}

void biheap_dump(biheap_t *self) {
    heap_dump(self->maxHeap);
    heap_dump(self->minHeap);
}


/*

Complexity: O(n log n), because I'm making O(N) requests to O(1) heaps

*/


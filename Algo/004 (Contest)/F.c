/*

ѕредставьте себе клетчатую окружность, состо€ющую из L клеток.  летки нумеруютс€ целыми
числами от 1 до L. Ќекоторые N клеток закрашены. ќкружность можно разрезать между любыми
двум€ клетками. ¬сего существует L различных разрезов. ѕолучивша€с€ полоска делитс€ на K
равных частей (L кратно K). \N{FOR ALL}i определим fi
, как количество закрашенных клеток в i-й части. ¬ам
нужно найти такой разрез, что
F = max
i=1..K
fi \N{MINUS SIGN} min
i=1..K
fi
минимально возможно.  роме того, вам нужно найти количество разрезов, на которых достигаетс€
максимум.
‘ормат входных данных
Ќа первой строке целые числа L, N, K, (K > 2). √арантируетс€, что L делитс€ на K.
Ќа второй строке N различных целых чисел от 1 до L Ч номера закрашенных клеток.
‘ормат выходных данных
Ќа первой строке выведите минимальное значение F и количество разрезов с таким F. Ќа второй
строке выведите любое x от 1 до L, что, если разрезать окружность между клетками x и x + 1, а
после чего посчитать F, то получитс€ минимальное значение.
ќграничени€: N, K 6 500 000, L 6 1018
.
ѕримеры
стандартный ввод стандартный вывод
10 1 5
7

1 10
10

10 4 2
3 7 5 2

0 4
3
«амечание
¬ первом примере подойдет любой разрез.
¬о втором примере разрезать можно после 3, 4, 8, 9 клеток.

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


static heap_item_t segSize_ = 0;  // I'm sorry, but I *really* need this for my comparator
// (The other solution requires usage of non-standard qsort_s/qsort_r)


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

static const heap_item_t MAX_NUM = 1000000000000000009;

//--------------------------------------------------------------------------------

static int cmp_(const void *a, const void *b);

heap_t *heap_new(unsigned capacity, int isMin);

void heap_free(heap_t *self);

void heap_push(heap_t *self, heap_item_t value, unsigned reqInd);

void heap_top(heap_t *self, heap_item_t *value);

void heap_pop(heap_t *self, heap_item_t *value);

void heap_decreaseKey(heap_t *self, unsigned reqInd, heap_item_t delta);

void heap_increaseKey(heap_t *self, unsigned reqInd, heap_item_t delta);

static void heap_swap(heap_t *self, unsigned a, unsigned b);

static void heap_siftDown(heap_t *self, unsigned ind);

static void heap_siftUp(heap_t *self, unsigned ind);

void heap_dump(heap_t *self);


biheap_t *biheap_new(unsigned capacity);

void biheap_free(biheap_t *self);

void biheap_push(biheap_t *self, heap_item_t value);

void biheap_popMin(biheap_t *self, heap_item_t *value);

void biheap_popMax(biheap_t *self, heap_item_t *value);

void biheap_getMin(biheap_t *self, heap_item_t *value);

void biheap_getMax(biheap_t *self, heap_item_t *value);

void biheap_decKey(biheap_t *self, unsigned key, heap_item_t delta);

void biheap_incKey(biheap_t *self, unsigned key, heap_item_t delta);

void biheap_dump(biheap_t *self);

//--------------------------------------------------------------------------------


int main() {
    int res = 0;

    heap_item_t l = 0;
    unsigned n = 0;
    unsigned k = 0;
    res = scanf("%lld %u %u", &l, &n, &k);
    assert(res == 3);

    heap_item_t segSize = l / k;

    segSize_ = segSize;

    heap_item_t *arr = (heap_item_t *)calloc(n, sizeof(arr[0]));  // Here we store the indices at which the cells are filled

    heap_item_t *buf = (heap_item_t *)calloc(k, sizeof(buf[0]));  // A temporary buffer for initial filled cell counters per block

    for (unsigned i = 0; i < n; ++i) {
        res = scanf("%lld", &arr[i]);
        assert(res == 1);

        arr[i]--;

        assert(arr[i] / segSize < k);

        buf[arr[i] / segSize]++;
    }

    biheap_t *biheap = biheap_new(k + 1);

    for (unsigned i = 0; i < k; ++i) {
        biheap_push(biheap, buf[i]);
    }

    qsort(arr, n, sizeof(arr[0]), cmp_);  // We sort arr based on the order the elements appear in their corresponding blocks

    heap_item_t minDelta = MAX_NUM;
    heap_item_t minCnt = 0;
    heap_item_t minStart = 0;

    //biheap_dump(biheap);

    for (unsigned i = 1; i < n; ++i) {
        //printf("> %lld\n", arr[i - 1]);
        biheap_decKey(biheap, arr[i - 1] / segSize, 1);
        biheap_incKey(biheap, (arr[i - 1] / segSize + k - 1) % k, 1);

        if (arr[i] % segSize == arr[i - 1] % segSize) {
            continue;
        }

        heap_item_t curMax = 0, curMin = 0;
        heap_item_t curMod = arr[i] % segSize;
        heap_item_t curDelta = 0;

        biheap_getMin(biheap, &curMin);
        biheap_getMax(biheap, &curMax);

        curDelta = curMax - curMin;

        //printf("> [%u] : %lld\n", i, curDelta);

        if (curDelta < minDelta) {
            minDelta = curDelta;
            minStart = arr[i - 1] % segSize;
            minCnt = 0;
        }

        if (curDelta == minDelta) {
            minCnt += curMod - arr[i - 1] % segSize;

            //printf("= %lld\n", minCnt);
        }
    }

    //printf("> %lld\n", arr[n - 1]);

    biheap_decKey(biheap, arr[n - 1] / segSize, 1);
    biheap_incKey(biheap, (arr[n - 1] / segSize + k - 1) % k, 1);
    heap_item_t curMax = 0, curMin = 0, curDelta = 0;
    biheap_getMin(biheap, &curMin);
    biheap_getMax(biheap, &curMax);

    curDelta = curMax - curMin;

    if (curDelta < minDelta) {
        minDelta = curDelta;
        minStart = arr[n - 1] % segSize;
        minCnt = 0;
    }

    if (curDelta == minDelta) {
        minCnt += arr[0] % segSize + segSize - arr[n - 1] % segSize;

        //printf("= %lld\n", minCnt);
    }

    if (minCnt == segSize + 1) {
        minCnt--;
    }

    printf("%lld %lld\n%lld\n", minDelta, minCnt * k, minStart + 1);

    free(arr);
    free(buf);
    biheap_free(biheap);

    return 0;
}

//--------------------------------------------------------------------------------

/*
  Heap and biheap are copied from G, 'docs' for them can be found at https://codeforces.com/group/R3IJoiTue4/contest/296750/submission/96330898
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

    self->data[self->lookupByRequest[reqInd]] -= delta;

    heap_siftUp(self, self->lookupByRequest[reqInd]);
}

void heap_increaseKey(heap_t *self, unsigned reqInd, heap_item_t delta) {  // It seems like this should work
    assert(self != NULL);
    assert(reqInd < self->capacity);

    self->data[self->lookupByRequest[reqInd]] += delta;

    heap_siftDown(self, self->lookupByRequest[reqInd]);
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
        heap_item_t min3 = self->data[ind];

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

//--------------------------------------------------------------------------------

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

    heap_top(self->minHeap, value);
    heap_decreaseKey(self->maxHeap, target, MAX_NUM - *value);

    heap_pop(self->maxHeap, NULL);
    heap_pop(self->minHeap, NULL);
}

void biheap_popMax(biheap_t *self, heap_item_t *value) {
    assert(self != NULL);

    unsigned target = self->maxHeap->lookupByValue[1];

    heap_top(self->maxHeap, value);

    heap_decreaseKey(self->minHeap, target, MAX_NUM - *value);

    heap_pop(self->minHeap, NULL);
    heap_pop(self->maxHeap, NULL);
}

void biheap_getMin(biheap_t *self, heap_item_t *value) {
    assert(self != NULL);

    heap_top(self->minHeap, value);
}

void biheap_getMax(biheap_t *self, heap_item_t *value) {
    assert(self != NULL);

    heap_top(self->maxHeap, value);
}

void biheap_decKey(biheap_t *self, unsigned key, heap_item_t delta) {
    assert(self != 0);
    assert(key < self->size);
    assert(delta > 0);

    heap_decreaseKey(self->minHeap, key, delta);
    heap_increaseKey(self->maxHeap, key, delta);
}

void biheap_incKey(biheap_t *self, unsigned key, heap_item_t delta) {
    assert(self != 0);
    assert(key < self->size);
    assert(delta > 0);

    heap_increaseKey(self->minHeap, key, delta);
    heap_decreaseKey(self->maxHeap, key, delta);
}

void biheap_dump(biheap_t *self) {
    heap_dump(self->maxHeap);
    heap_dump(self->minHeap);
}

static int cmp_(const void *a, const void *b) {
    heap_item_t aVal = *(heap_item_t *)a % segSize_;
    heap_item_t bVal = *(heap_item_t *)b % segSize_;

    if (aVal < bVal) {  // Subtraction wouldn't work because the type size is alteady the biggest
        return -1;
    } else if (aVal == bVal) {
        return 0;
    } else {
        return 1;
    }
}


/*

Complexity: O(n log k), because we do a constant number of operations on a (bi)heap with k elements in an n-loop

*/



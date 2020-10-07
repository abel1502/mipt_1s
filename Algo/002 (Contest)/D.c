/**
 * @file
 * @warning This heap implementation assumes zero to be an invalid element
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


typedef unsigned long long heap_item_ct;

typedef struct heap_cs heap_ct;

struct heap_cs {
    size_t size;
    size_t capacity;
    heap_item_ct data[1];
};

heap_ct *heap_new(size_t capacity);

void heap_free(heap_ct *self);

void heap_push(heap_ct *self, heap_item_ct value);

void heap_top(heap_ct *self, heap_item_ct *value);

void heap_pop(heap_ct *self, heap_item_ct *value);

static void heap_swap(heap_ct *self, size_t a, size_t b);

static void heap_siftDown(heap_ct *self, size_t ind);

static void heap_siftUp(heap_ct *self, size_t ind);

void heap_dump(heap_ct *self);

//--------------------------------------------------------------------------------

int main() {
    size_t n = 0;
    scanf("%zu", &n);

    heap_ct *heap = heap_new(n);

    for (size_t i = 0; i < n; ++i) {
        heap_item_ct cur = 0;
        scanf("%u", &cur);

        heap_push(heap, cur);
    }

    unsigned long long ans = 0; //printf("!!!\n");

    while (heap->size > 1) {
        heap_item_ct cur1 = 0, cur2 = 0;

        //printf("<%llu>\n", ans);
        //heap_dump(heap);

        heap_pop(heap, &cur1);
        heap_pop(heap, &cur2);

        //printf("<%u, %u>\n", cur1, cur2);

        ans += cur1 + cur2;

        heap_push(heap, cur1 + cur2);

        //printf("%u ", cur);
    }

    //printf("%llu\n", ans);
    printf("%llu.%2llu\n", ans / 100, ans % 100);

    heap_free(heap);

    return 0;
}

//--------------------------------------------------------------------------------

heap_ct *heap_new(size_t capacity) {
    heap_ct *self = (heap_ct *)calloc(1, sizeof(heap_ct) + capacity * sizeof(heap_item_ct));
    assert(self != NULL);

    self->capacity = capacity;
    self->size = 0;

    return self;
}

void heap_free(heap_ct *self) {
    free(self);
}

void heap_push(heap_ct *self, heap_item_ct value) {
    assert(self != NULL);
    assert(self->size + 1 <= self->capacity);

    self->size++;

    self->data[self->size] = value;

    heap_siftUp(self, self->size);
}

void heap_top(heap_ct *self, heap_item_ct *value) {
    assert(self != NULL);
    assert(value != NULL);

    assert(self->size > 0);

    *value = self->data[1];
}

void heap_pop(heap_ct *self, heap_item_ct *value) {
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

static void heap_swap(heap_ct *self, size_t a, size_t b) {
    assert(self != NULL);
    assert(1 <= a && a <= self->size);
    assert(1 <= b && b <= self->size);

    heap_item_ct tmp = self->data[a];
    self->data[a] = self->data[b];
    self->data[b] = tmp;
}

static void heap_siftDown(heap_ct *self, size_t ind) {
    assert(self != NULL);
    assert(1 <= ind && ind <= self->size);

    while (2 * ind <= self->size) {
        size_t minInd = ind;
        heap_item_ct min3 = self->data[ind];

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

static void heap_siftUp(heap_ct *self, size_t ind) {
    assert(self != NULL);
    assert(1 <= ind && ind <= self->size);

    while (ind > 1 && self->data[ind] < self->data[ind / 2]) {
        heap_swap(self, ind, ind / 2);
        ind /= 2;
    }
}

void heap_dump(heap_ct *self) {
    printf("heap_ct [0x%p] {\n", self);
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
                for (size_t j = 0; j < sizeof(heap_item_ct); ++j) {
                    printf("%02X", ((unsigned char *)(self->data + i))[j]);
                }
                printf(" ");

                printf("(");
                printf("%u", self->data[i]);
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


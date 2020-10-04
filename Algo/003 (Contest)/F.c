/*

Напишите программу, которая для заданного массива A=?a1,a2,…,an? находит количество пар (i,j) таких, что i<j и ai>aj. Обратите внимание на то, что ответ может не влезать в int.

Входные данные
Первая строка входного файла содержит натуральное число n (1?n?100000) — количество элементов массива. Вторая строка содержит n попарно различных элементов массива A — целых неотрицательных чисел, не превосходящих 109.

Выходные данные
В выходной файл выведите одно число — ответ на задачу.

Примеры
входные данные
5
6 11 18 28 31
выходные данные
0
входные данные
5
179 4 3 2 1
выходные данные
10

*/

#define REDIRECT_STDIO 1

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

// For debug
// #define assert(stmt) if (!(stmt)) {exit(__LINE__);}

//--------------------------------------------------------------------------------

typedef unsigned long long item_t;

//--------------------------------------------------------------------------------

static void merge_(item_t *base, size_t left, size_t middle, size_t right, item_t *mergeBuf, unsigned long long *inversions);

static void msort_(item_t *base, size_t left, size_t right, item_t *mergeBuf, unsigned long long *inversions);

void msort(item_t *base, size_t size, unsigned long long *inversions);

/*void logArr(item_t *base, size_t size) {
    for (size_t i = 0; i < size; ++i) {
        printf("%llu ", base[i]);
    }
    printf("\n");
}*/

//--------------------------------------------------------------------------------

int main() {
    #if REDIRECT_STDIO
    FILE *fin  = fopen("inverse.in",  "r");
    FILE *fout = fopen("inverse.out", "w");

    assert(fin  != NULL);
    assert(fout != NULL);
    #else
    FILE *fin  = stdin;
    FILE *fout = stdout;
    #endif // REDIRECT_STDIO

    int res = 0;

    unsigned n = 0;

    res = fscanf(fin, "%u", &n);
    assert(res == 1);

    item_t *arr = (item_t *)calloc(n, sizeof(item_t));
    assert(arr != NULL);

    for (unsigned i = 0; i < n; ++i) {
        res = fscanf(fin, "%llu", &arr[i]);
        assert(res == 1);
    }

    unsigned long long inversions = 0;

    msort(arr, n, &inversions);

    //logArr(arr, n);
    fprintf(fout, "%llu\n", inversions);

    free(arr);


    #if REDIRECT_STDIO
    fclose(fin);
    fclose(fout);
    #endif // REDIRECT_STDIO

    return 0;
}

//--------------------------------------------------------------------------------

void msort(item_t *base, size_t size, unsigned long long *inversions) {
    assert(base != NULL);
    assert(inversions != NULL);

    if (size == 0) {
        return;
    }

    item_t *mergeBuf = (item_t *)calloc(size, sizeof(item_t));
    assert(mergeBuf != NULL);

    msort_(base, 0, size - 1, mergeBuf, inversions);

    free(mergeBuf);
}

//--------------------------------------------------------------------------------

static void msort_(item_t *base, size_t left, size_t right, item_t *mergeBuf, unsigned long long *inversions) {
    assert(base != NULL);
    assert(mergeBuf != NULL);
    assert(inversions != NULL);

    if (left >= right) {
        return;
    }

    size_t middle = (left + right) / 2;

    //printf("> ");
    //logArr(base + left, right - left + 1);

    msort_(base, left, middle, mergeBuf, inversions);
    msort_(base, middle + 1, right, mergeBuf, inversions);

    //printf("= ");
    //logArr(base + left, right - left + 1);
    //printf("[%zu %zu %zu]\n", left, middle, right);

    merge_(base, left, middle, right, mergeBuf, inversions);

    //printf("< ");
    //logArr(base + left, right - left + 1);

}

//--------------------------------------------------------------------------------

// The two subarrays are [left..middle] and [middle+1..right]. God, bi-inlusive segments are confusing...
static void merge_(item_t *base, size_t left, size_t middle, size_t right, item_t *mergeBuf, unsigned long long *inversions) {
    assert(base != NULL);
    assert(mergeBuf != NULL);
    assert(left <= middle && middle <= right);
    assert(inversions != NULL);

    item_t *leftBuf  = mergeBuf + left;
    item_t *rightBuf = mergeBuf + middle + 1;

    size_t leftSize  = middle + 1 - left;
    size_t rightSize = right - middle;

    //memcpy(leftBuf, base + left, leftSize + 1);
    //memcpy(rightBuf, base + middle + 1, rightSize + 1);
    memcpy(leftBuf, base + left, (right + 1 - left) * sizeof(item_t));

    size_t leftInd = 0, rightInd = 0, baseInd = left;

    while (leftInd < leftSize && rightInd < rightSize) {
        if (leftBuf[leftInd] <= rightBuf[rightInd]) {
            base[baseInd++] = leftBuf[leftInd++];
        } else {
            *inversions += leftSize - leftInd;
            base[baseInd++] = rightBuf[rightInd++];
        }
        //printf("(%zu %zu)\n", leftInd, rightInd);
    }

    while (leftInd < leftSize) {
        base[baseInd++] = leftBuf[leftInd++];
    }

    while (rightInd < rightSize) {
        base[baseInd++] = rightBuf[rightInd++];
    }

    assert(baseInd == right + 1);
}

//--------------------------------------------------------------------------------

/*

Complexity: O(n log n) , because mergesort

*/


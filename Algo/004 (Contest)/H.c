/*

Дан массив неотрицательных целых 64-битных чисел. Количество чисел не больше 106. Отсортировать массив методом поразрядной сортировки LSD по байтам.

Входные данные
В первой строке вводится количество чисел в массиве N. На следующей строке через пробел вводятся N неотрицательных чисел.

Выходные данные
Выведите этот массив, отсортированный в порядке возрастания, в одну строчку через пробел.

Пример
входные данные
3
4 1000000 7
выходные данные
4 7 1000000

*/

#ifdef ALGO_LOCAL
#define __USE_MINGW_ANSI_STDIO 1
#endif

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

//#define T fprintf(stderr, "[Reached #%d]\n", __LINE__);


typedef unsigned long long item_t;


int radix_sort(item_t *arr, unsigned length);


int main() {
    int res = 0;

    unsigned n = 0;
    res = scanf("%u", &n);
    assert(res == 1);

    item_t *arr = (item_t *)calloc(2 * n, sizeof(arr[0]));  // This is the trickiest part - arr is two buffers, because radix isn't an in-place sort in terms of memory
                                                            // I tried to have something like arr[2][n] based on pointers, but it didn't really work, so I stuck to the simple approach

    for (unsigned i = 0; i < n; ++i) {
        res = scanf("%llu", &arr[i]);
        assert(res == 1);
    }

    int sortedArr = radix_sort(arr, n);

    for (unsigned i = 0; i < n; ++i) {
        printf("%llu ", arr[sortedArr + i]);
    }
    printf("\n");

    free(arr);

    return 0;
}


// Radix sort was explain on one seminar - we count-sort every byte from low to high, and since we do it in a stable way, it does what we want
int radix_sort(item_t *arr, unsigned length) {
    unsigned *counts = (unsigned *)calloc(256, sizeof(counts[0]));

    int curBuf = 0;

    for (unsigned char sortedByte = 0; sortedByte < sizeof(item_t); ++sortedByte) {
        memset(counts, 0, 256 * sizeof(counts[0]));

        /*printf("Byte #%hhu\n", sortedByte);
        for (unsigned i = 0; i < length; ++i) {
            printf("%llu ", arr[curBuf + i]);
        } printf("\n");

        for (unsigned i = 0; i < length; ++i) {
            printf("%llu ", arr[length - curBuf + i]);
        } printf("\n\n");*/

        unsigned char shift = sortedByte << 3;  // bytes to bits

        for (unsigned i = 0; i < length; ++i) {
            //printf("%lld[%hhu] = %02x\n", arr[i], sortedByte, (char)((arr[i] >> shift) & 0xff));
            counts[(arr[curBuf + i] >> shift) & 0xff]++;
        }

        unsigned carry = counts[0];
        counts[0] = 0;
        for (unsigned i = 1; i < 256; ++i) {
            unsigned tmp = counts[i];
            counts[i] = carry + counts[i - 1];
            carry = tmp;
        }

        //printf("%p %p\n", sorted, counts);

        for (unsigned i = 0; i < length; ++i) {
            /*for (unsigned j = 0; j < 256; ++j) {
                printf("> counts[%02x] = %u\n", j, counts[j]);
            }*/
            unsigned curByte = (arr[curBuf + i] >> shift) & 0xff;
            //printf("counts[%02x] = %u\n", curByte, counts[curByte]);
            arr[length - curBuf + counts[curByte]++] = arr[curBuf + i];
        }

        curBuf = length - curBuf;
    }

    free(counts);

    return curBuf;
}


/*

Complexity: O(n), because we repeat a count sort on n elements from a 256-long alphabet
            eight times - which is O(8 * (n + 256))===O(n) - as expected from radix

*/


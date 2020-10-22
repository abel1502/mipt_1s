/*

ƒаны неотрицательные целые числа N, K и массив целых чисел из диапазона [0,109] размера N. “ребуетс€ найти K-ю пор€дковую статистику, т.е. напечатать число, которое бы сто€ло на позиции с индексом K [0,N1] в отсортированном массиве.

Ќапишите нерекурсивный алгоритм. “ребовани€ к дополнительной пам€ти: O(1). “ребуемое среднее врем€ работы: O(N).

¬ходные данные
¬ первой строке записаны N и K.

¬ N последующих строках записаны числа последовательности.

¬ыходные данные
K-€ пор€дкова€ статистика.

ѕримеры
входные данные
4 3
3
5
7
9
выходные данные
9
входные данные
10 0
3
6
5
7
2
9
8
10
4
1
выходные данные
1

*/

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>


//#define ALGO_DEBUG


typedef unsigned item_t;

item_t kth_elem(item_t *arr, unsigned num, unsigned k);

unsigned partition(item_t *arr, unsigned left, unsigned right, item_t doublePivot);


int main() {
    int res = 0;

    unsigned n = 0, k = 0;
    res = scanf("%u %u", &n, &k);
    assert(res == 2);

    item_t *arr = (item_t *)calloc(n, sizeof(item_t));

    for (unsigned i = 0; i < n; ++i) {
        res = scanf("%u", &arr[i]);
        assert(res == 1);
    }

    printf("%u\n", kth_elem(arr, n, k));

    free(arr);

    return 0;
}


/*
  Calculates the k'th element in the sorted array.

  Works the same as quicksort, except only repeats for one part instead of both.
  (It only chooses the part where the desired k'th element resides, since the order
  of the elements within the other one doesn't really matter.) Because of this
  it works in linear time instead of n log n.
*/
item_t kth_elem(item_t *arr, unsigned num, unsigned k) {
    assert(arr != NULL);
    assert(k < num);

    unsigned left = 0, right = num - 1;  // Borders of the segment we're 'sorting'

    while (left < right) {
        item_t arrMax = 0, arrMin = (item_t)-1;

        for (unsigned i = left; i <= right; ++i) {
            if (arr[i] > arrMax) {
                arrMax = arr[i];
            }

            if (arr[i] < arrMin) {
                arrMin = arr[i];
            }
        }

        // We find the min and max within this segment since I need them to exclude one case in which my
        // ingenious idea wouldn't work, but we may also use them to provide an exit cond.

        if (left == k) {  // This is our exit condition
            return arrMin;
        } else if (right == k) {
            return arrMax;
        }

        // A special crotch to exclude the cases where the oddified double pivot will be too big
        if (arrMax == arrMin)
            return arrMax;

        // Intentionally made odd
        item_t doublePivot = (arrMax + arrMin) | 1;

        unsigned middle = partition(arr, left, right, doublePivot);

        #if defined(ALGO_LOCAL) && defined(ALGO_DEBUG)
        for (unsigned i = 0; i < num; ++i) {
            printf("%u ", arr[i]);
        }
        printf("\n[%u %u %u]\n", left, middle, right);
        printf("pivot * 2 = %u\n", doublePivot);
        system("pause");
        #endif

        // We're only intrested in the segment containing the k'th element
        if (middle < k) {
            left = middle + 1;
        } else {
            right = middle;
        }
    }

    return arr[left];
}

/*
  Essentially a quicksort partition, but insted of pivot it uses an odd 'double pivot'
  (which is equivalent to half-integral pivot) to make sure the array contains no elements equal to the pivot.
  Maybe we don't need it, but I've implemented this just to be sure my program won't enter an infinite loop
  with elements equal to the pivot in the array.
*/
unsigned partition(item_t *arr, unsigned left, unsigned right, item_t doublePivot) {
    assert(arr != NULL);
    assert((doublePivot & 1) == 1);  // We require an odd double pivot to exclude the possibility of a match between the pivot and some element

    unsigned i = left;
    unsigned j = right;

    while (1) {
        while (i <= right && arr[i] * 2 < doublePivot) ++i;

        while (j >= left && j != (unsigned)-1 && doublePivot < arr[j] * 2) --j;

        if (i >= j) break;

        item_t tmp = arr[i];
        arr[i] = arr[j];
        arr[j] = tmp;

        i++;
        j--;
    }

    return j;
}


/*

Complexity: O(1) memory, O(N) time - was proven in class already :)

*/

/*

���� ��������������� ����� ����� N, K � ������ ����� ����� �� ��������� [0,109] ������� N. ��������� ����� K-� ���������� ����������, �.�. ���������� �����, ������� �� ������ �� ������� � �������� K [0,N1] � ��������������� �������.

�������� ������������� ��������. ���������� � �������������� ������: O(1). ��������� ������� ����� ������: O(N).

������� ������
� ������ ������ �������� N � K.

� N ����������� ������� �������� ����� ������������������.

�������� ������
K-� ���������� ����������.

�������
������� ������
4 3
3
5
7
9
�������� ������
9
������� ������
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
�������� ������
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


item_t kth_elem(item_t *arr, unsigned num, unsigned k) {
    assert(arr != NULL);
    assert(k < num);

    unsigned left = 0, right = num - 1;

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

        if (left == k) {  // This is our exit condition
            return arrMin;
        } else if (right == k) {
            return arrMax;
        }

        // A special crotch to exclude the cases where the oddified double partition will be too big
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

        if (middle < k) {
            left = middle + 1;
        } else {
            right = middle;
        }
    }

    return arr[left];
}

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

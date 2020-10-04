/*

���� n �������� ������������ �������� Ai � m �������� ��������� �������� Bj. ��� ������� ����� ���� � �� �� ����� l. ����� ���� q �������� ���� (i,j), ����� �� ������ � ����� k, ��� max(Aik,Bjk) ���������. ���� ����� k ���������, ����� ������� �����.

������� ������
�� ������ ������ ����� n,m,l (1?n,m?900;1?l?3000). ��������� n ����� �������� �������� �������� Ai. ������ ������ ����������� ������������� l ���������. �������� ������� � ����� ����� �� 0 �� 105?1. ����� ����� m � �������� �������� Bj � ����� �� �������. ������� � �������� ������ ������� ���������� � 1. �� ��������� ������ ����� �������� q (1?q?n?m). ��������� q ����� �������� ���� ����� i,j (1?i?n, 1?j?m).

�������� ������
�������� q ����� �� 1 �� l � ������ �� �������.

������
������� ������
4 3 5
1 2 3 4 5
1 1 1 1 1
0 99999 99999 99999 99999
0 0 0 0 99999
5 4 3 2 1
99999 99999 99999 0 0
99999 99999 0 0 0
12
1 1
1 2
1 3
2 1
2 2
2 3
3 1
3 2
3 3
4 1
4 2
4 3
�������� ������
3
4
3
5
4
3
1
2
2
4
4
3

*/

/*

    ========================
           IMPORTANT!
    ========================
    This program is written and works in pure C,
    but works too slowly in the contest due to
    the old version of the C compiler. Using the
    C++ compiler solves the issue, and this approach
    is approved by I.R. Dedinsky (we have asked him
    about it on a seminar, you may a well, if you
    don't trust me).

*/


#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>


unsigned solve(int *curA, int *curB, unsigned length);
int max(int x, int y);


int main() {
    int res = 0;
    unsigned n = 0, m = 0, l = 0;

    res = scanf("%u %u %u", &n, &m, &l);
    assert(res == 3);

    // For convenience purposes,
    typedef int matrix[l];

    matrix *A = (matrix *)calloc(n, sizeof(matrix));
    matrix *B = (matrix *)calloc(m, sizeof(matrix));

    assert(A != NULL);
    assert(B != NULL);

    for (unsigned i = 0; i < n; ++i) {
        for (unsigned j = 0; j < l; ++j) {
            res = scanf("%d", &A[i][j]);
            assert(res == 1);
        }
    }

    for (unsigned i = 0; i < m; ++i) {
        for (unsigned j = 0; j < l; ++j) {
            res = scanf("%d", &B[i][j]);
            assert(res == 1);
        }
    }

    unsigned q = 0;

    res = scanf("%u", &q);
    assert(res == 1);

    for (unsigned _ = 0; _ < q; ++_) {  // This is the way to name unused loop variables taught to me by D.P. Kirienko
        unsigned i = 0, j = 0;

        res = scanf("%u %u", &i, &j);
        assert(res == 2);
        assert(i >= 1);
        assert(j >= 1);

        i--;  // Due to 1-indexing
        j--;

        unsigned k = solve(A[i], B[j], l);  // Sadly, there's no way to specify matrix * as argument type for solve, so we have to pass l as well
        k++;

        printf("%u\n", k);
    }

    free(A);
    free(B);

    return 0;
}

int max(int x, int y) {
    if (x > y) {
        return x;
    }

    return y;
}

unsigned solve(int *curA, int *curB, unsigned length) {
    unsigned left = 0, right = length;

    while (left + 1 < right) {
        unsigned middle = (left + right) / 2;

        if (curA[middle] < curB[middle]) {
            left  = middle;
        } else {
            right = middle;
        }
    }

    if (right == length) {
        return left;
    }

    if (max(curA[left], curB[left]) < max(curA[right], curB[right])) {
        return left;
    }

    return right;
}

/*

Complexity: O(n * l + m * l + q * log l) - input + input + binary search for every request

*/


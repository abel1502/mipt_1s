/*

Ќа числовой пр€мой окрасили N отрезков. »звестны координаты левого и правого концов каждого отрезка [Li,Ri]. Ќайти сумму длин частей числовой пр€мой, окрашенных ровно в один слой. N10000. Li,Ri Ч целые числа в диапазоне [0,109].

¬ходные данные
¬ первой строке записано количество отрезков. ¬ каждой последующей строке через пробел записаны координаты левого и правого концов отрезка.

¬ыходные данные
¬ыведите целое число Ч длину окрашенной в один слой части.

ѕример
входные данные
3
1 4
7 8
2 5
выходные данные
3

*/


#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>


typedef struct point_s point_t;

struct point_s {
    unsigned pos;
    int delta;
};


int cmpPoints(const void *a, const void *b);


int main() {
    int res = 0;

    unsigned n = 0;
    res = scanf("%u", &n);
    assert(res == 1);

    // TODO: In case of TL switch to point_t **. (I'm not sure which is slower: n calloc's or n log n extra 4-byte swaps)
    point_t *points = (point_t *)calloc(n * 2, sizeof(point_t));
    assert(points != NULL);


    for (unsigned i = 0; i < n; ++i) {
        res = scanf("%u %u", &points[2 * i].pos, &points[2 * i + 1].pos);
        assert(res == 2);

        points[2 * i].delta = 1;
        points[2 * i + 1].delta = -1;
    }

    qsort(points, 2 * n, sizeof(point_t), cmpPoints);

    int curLevel = 0;
    unsigned ans = 0;
    unsigned lastPos = points[0].pos;

    for (unsigned i = 0; i < 2 * n; ++i) {
        //printf("[%u] %i\n", points[i].pos, points[i].delta);

        //printf("> %i * %i\n", curLevel, points[i].pos - lastPos);
        if (curLevel == 1) {
            ans += points[i].pos - lastPos;
        }

        lastPos = points[i].pos;

        curLevel += points[i].delta;
    }

    assert(curLevel == 0);

    free(points);

    printf("%u\n", ans);

    return 0;
}


int cmpPoints(const void *a, const void *b) {
    point_t *aPt = (point_t *)a;
    point_t *bPt = (point_t *)b;

    // Sadly, we can't simply subtract due to overflows
    if (aPt->pos < bPt->pos) {
        return -1;
    } else if (aPt->pos == bPt->pos) {
        return 0;
    } else {
        return 1;
    }
}

/*

Complexity: O(n log n), because we quick sort n elements with a constant comparator, and the rest is linear

*/

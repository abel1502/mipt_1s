/*

√ерой по имени ћагина сражаетс€ с группой из n монстров с помощью легендарного топора, известного как ярость Ѕитвы.  аждый из монстров имеет ai очков здоровь€.  аждым ударом топора ћагина уменьшает здоровье того, кого он ударил, на p очков, при этом уменьша€ здоровье всех остальных монстров на q очков. ћонстр умирает, когда у него остаетс€ 0 или менее очков здоровь€. ћагина хочет при каждом ударе выбирать цель таким образом, чтобы убить всех монстров за минимальное количество ударов. “ребуетс€ определить это количество.

¬ходные данные
¬ первой строке содержатс€ три целых числа через пробел: n, p и q (1?n?200000, 1?q?p?109) Ч количество монстров, урон по цели и урон по всем остальным соответственно.

¬о второй строке содержатс€ n целых чисел через пробел: ai (1?ai?109) Ч количество очков здоровь€ у каждого из монстров.

¬ыходные данные
¬ыведите единственное целое число Ч минимальное количество ударов, за которое ћагина сможет убить всех монстров.

ѕримеры
входные данные
2 3 2
5 5
выходные данные
2
входные данные
3 5 3
17 13 14
выходные данные
5
входные данные
1 6 4
19
выходные данные
4
входные данные
2 5 5
22 23
выходные данные
5

*/

// Apparently the test server doesn't define NDEBUG. Fine, I'll do it myself)
#define NDEBUG

#ifndef NDEBUG
#define __USE_MINGW_ANSI_STDIO 1
#endif // NDEBUG

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

// Now, I know this is a macro and isn't appreciated, but maybe it's okay here, since it's for debug only?
#ifndef NDEBUG
#define DBG(fmt, ...) printf("[%s]: " fmt "\n", __func__, ##__VA_ARGS__)
#else
#define DBG(fmt, ...)
#endif // NDEBUG

//--------------------------------------------------------------------------------

typedef long long item_t;

//--------------------------------------------------------------------------------

// `Swipe` and `target` are hearthstone-based terms for damage to everyone and to a single target;
// answer is item_t because it is proportional to max(arr) / swipe, which may be as big as item_t
item_t check(item_t answer, item_t *arr, size_t size, item_t swipe, item_t target);

item_t solve(item_t *arr, size_t size, item_t swipe, item_t target);

int compareItems(const void *a, const void *b);

/*void logArr(item_t *base, size_t size) {
    for (size_t i = 0; i < size; ++i) {
        printf("%lld ", base[i]);
    }
    printf("\n");
}*/

//--------------------------------------------------------------------------------

int main() {
    int res = 0;

    unsigned n = 0; // I would prefer to have this as size_t, but the server-side compiler seems to fail on %zu
    item_t p = 0, q = 0;

    res = scanf("%u %lld %lld", &n, &p, &q);
    assert(res == 3);

    size_t size = n;

    item_t *arr = (item_t *)calloc(size, sizeof(item_t));

    for (size_t i = 0; i < size; ++i) {
        res = scanf("%lld", &arr[i]);
        assert(res == 1);
    }

    qsort(arr, size, sizeof(item_t), compareItems);

    //logArr(arr, n);

    printf("%lld\n", solve(arr, size, q, p));

    free(arr);

    return 0;
}

//--------------------------------------------------------------------------------

item_t check(item_t answer, item_t *arr, size_t size, item_t swipe, item_t target) {
    assert(arr != NULL);
    assert(size > 0);
    assert(0 < swipe && swipe <= target);

    DBG("Checking %lld:", answer);

    item_t baseDmg = swipe * answer;  // Base damage dealt to eveyone
    item_t extraDmg  = target - swipe;  // Extra damage, applicable to one target at a time

    DBG("Base dmg:  %lld", baseDmg);
    DBG("Extra dmg: %lld", extraDmg);

    if (extraDmg == 0) {
        return baseDmg - arr[0];
    }

    for (size_t i = 0; i < size && answer >= 0 && arr[i] > baseDmg; ++i) {
        DBG("[%lld] -> [%lld], needs <%lld> targeted hits", arr[i], arr[i] - baseDmg, (arr[i] - baseDmg + extraDmg - 1) / extraDmg);
        answer -= (arr[i] - baseDmg + extraDmg - 1) / extraDmg;
    }

    return answer;
}

//--------------------------------------------------------------------------------

item_t solve(item_t *arr, size_t size, item_t swipe, item_t target) {
    assert(arr != NULL);
    assert(size > 0);
    assert(0 < swipe && swipe <= target);

    item_t left = 0, right = arr[0] / swipe + 2;

    while (left + 1 < right) {
        DBG("[%lld, %lld]", left, right);

        item_t middle = (left + right) / 2;

        if (check(middle, arr, size, swipe, target) < 0) {
            left  = middle;
        } else {
            right = middle;
        }
    }

    return right;
}

//--------------------------------------------------------------------------------

int compareItems(const void *a, const void *b) {
    // return (int)((*(item_t *)b - *(item_t *)a) >> ((sizeof(item_t) - sizeof(int)) * 8));

    // While the above is somewhat cleaner, it looks a lot more confusing, so I'll probably prefer a different approach

    if (*(item_t *)a < *(item_t *)b) {
        return 1;
    } else if (*(item_t *)a == *(item_t *)b) {
        return 0;
    } else {
        return -1;
    }
}

//--------------------------------------------------------------------------------

/*

Complexity: O(n * log(max({ai}) / q)), because I do a binary search over max({ai})/q possible answers with a linear checker

*/

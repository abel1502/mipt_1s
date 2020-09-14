#include <stdio.h>
#include <stdlib.h>

typedef unsigned int uint;

int check(uint m, uint k, uint n, uint *v) {
    uint cur = m;
    uint cnt = 0;
    for (uint i = 0; i < n; ++i) {
        if (cur < v[i]) {
            cur = m;
            cnt++;
        }
        if (cur >= v[i]) {
            cur -= v[i];
        } else {
            return 2;
        }
    }
    if (cur < m) {
        cnt++;
    }
    if (cnt <= k) {
        return 0;
    }
    return 1;
}

int main() {
    uint k = 0, n = 0;
    uint *v = NULL;

    scanf("%u %u", &k, &n);
    v = (uint *)malloc(sizeof(uint) * n);
    for (uint i = 0; i < n; ++i) {
        scanf("%u", v + i);
    }

    uint low = 0, high = 1;
    while (check(high, k, n, v) > 0) {
        high *= 2;
    }

    while (low + 1 < high) {
        uint middle = (low + high) / 2;
        if (check(middle, k, n, v) > 0) {  // ?
            low = middle;
        } else {
            high = middle;
        }
    }

    printf("%u\n", high);

    free(v);

    return EXIT_SUCCESS;
}

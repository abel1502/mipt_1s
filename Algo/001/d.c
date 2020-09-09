#include <stdio.h>

int main() {
    unsigned long long u, l;
    scanf("%llu%llu", &u, &l);
    printf("%llu >> %llu = %llu\n", u, l, u >> l);

    return 0;
}

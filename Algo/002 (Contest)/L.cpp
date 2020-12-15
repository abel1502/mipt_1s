#include <cstdio>
#include <cassert>
#include <cstdlib>
#include <algorithm>
#include <cstring>


typedef unsigned long long hash_t;


struct PrefixHashes {
    static const hash_t MOD1 = 1000000007;
    static const hash_t MOD2 = 1000000009;
    static const hash_t BASE = 31;

    unsigned len;
    hash_t *values1;
    hash_t *values2;
    hash_t *basePows1;
    hash_t *basePows2;

    PrefixHashes *ctor(unsigned new_len, const char *str) {
        len = new_len;

        values1 = (hash_t *)calloc(len + 1, sizeof(hash_t));
        assert(values1);

        values2 = (hash_t *)calloc(len + 1, sizeof(hash_t));
        assert(values2);

        basePows1 = (hash_t *)calloc(len + 1, sizeof(hash_t));
        assert(basePows1);

        basePows2 = (hash_t *)calloc(len + 1, sizeof(hash_t));
        assert(basePows2);

        hash_t curHsh1 = 0;
        hash_t curHsh2 = 0;
        hash_t curPow1 = 1;
        hash_t curPow2 = 1;

        for (unsigned i = 0; i < len; ++i) {
            values1[i] = curHsh1;
            values2[i] = curHsh2;
            basePows1[i] = curPow1;
            basePows2[i] = curPow2;

            //printf("> %llu %llu\n", curHsh, curPow);

            curHsh1 = (curHsh1 * BASE % MOD1 + str[i] - 'a') % MOD1;
            curHsh2 = (curHsh2 * BASE % MOD2 + str[i] - 'a') % MOD2;
            curPow1 = (curPow1 * BASE) % MOD1;
            curPow2 = (curPow2 * BASE) % MOD2;
        }

        values1[len] = curHsh1;
        values2[len] = curHsh2;
        basePows1[len] = curPow1;
        basePows2[len] = curPow2;

        return this;
    }

    void dtor() {
        free(values1);
        free(values2);
        free(basePows1);
        free(basePows2);
    }

    hash_t getHash1(unsigned start, unsigned size) {
        assert(start + size <= len);

        return (values1[start + size] + MOD1 - values1[start] * basePows1[size] % MOD1) % MOD1;
    }

    hash_t getHash2(unsigned start, unsigned size) {
        assert(start + size <= len);

        return (values2[start + size] + MOD2 - values2[start] * basePows2[size] % MOD2) % MOD2;
    }
};


unsigned binSearch(PrefixHashes *table, unsigned *ansStart) {
    unsigned left = 0;
    unsigned right = table->len + 1;

    struct item_t {
        hash_t hash1;
        hash_t hash2;
        unsigned ind;

        bool operator< (const item_t &other) {
            return hash1 < other.hash1 || (hash1 == other.hash1 && hash2 < other.hash2);
        }

        bool operator== (const item_t &other) {
            return hash1 == other.hash1 && hash2 == other.hash2;
        }
    };

    unsigned cnt = 0;
    item_t *hashes = (item_t *)calloc(table->len, sizeof(item_t));

    bool enough = false;

    while (left + 1 < right) {
        //printf("= %u %u\n", left, right);

        enough = false;

        unsigned middle = (left + right) / 2;

        cnt = table->len - middle + 1;

        for (unsigned start = 0; start < cnt; ++start) {
            hashes[start] = {table->getHash1(start, middle), table->getHash2(start, middle), start};
        }

        std::sort(hashes, hashes + cnt);

        for (unsigned i = 1; i < cnt; ++i) {
            if (hashes[i] == hashes[i - 1]) {
                *ansStart = hashes[i].ind;
                enough = true;

                break;
            }
        }

        if (enough) {
            left = middle;
        } else {
            right = middle;
        }
    }

    //printf("= %u %u\n", left, right);

    return left;
}


int main() {
    unsigned len = 0;
    char *str = (char *)calloc(100001, 1);

    int res = scanf("%100000s", str);
    assert(res == 1);

    len = strlen(str);

    PrefixHashes table{};
    table.ctor(len, str);

    unsigned ansStart = 0;
    unsigned ansLen = 0;

    ansLen = binSearch(&table, &ansStart);

    printf("%.*s", ansLen, str + ansStart);

    table.dtor();

    free(str);

    return 0;
}


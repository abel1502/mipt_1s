/*

Даны n отрезков на прямой. Пара отрезков называются вложенной, если отрезки не совпадают, и один отрезок содержит второй. Посчитать количество пар вложенных отрезков.

Входные данные
Целоы число n (1\N{LESS-THAN OR EQUAL TO}n\N{LESS-THAN OR EQUAL TO}300000) и n пар целых чисел 0\N{LESS-THAN OR EQUAL TO}li\N{LESS-THAN OR EQUAL TO}ri\N{LESS-THAN OR EQUAL TO}109.

Выходные данные
Одно число – количество пар вложенных отрезков.

Пример
входные данные
4
1 4
2 5
1 3
3 4
выходные данные
3

*/


#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <cstring>
#include <algorithm>  // I believe you have allowed this.


namespace abel {

    // OP must provide static methods ::forward, ::reverse, such that
    // for any x, y reverse(forward(x, y), y) == x, as well as
    // typename ::T and static const T ::neutral, which is, well,
    // neutral to ::forward and ::backward
    template <typename TRAITS>
    class FenwickTree {
    public:

        using T = typename TRAITS::T;
        static const T NEUTRAL = TRAITS::neutral;

        static FenwickTree *create() {
            FenwickTree *self = new FenwickTree();

            return self->ctor();
        }

        void destroy() {
            dtor();
            delete this;
        }

        FenwickTree *ctor() {
            buf = nullptr;
            size = 0;

            return this;
        }

        FenwickTree *ctor(unsigned new_size) {
            size = new_size;

            buf = (T *)calloc(size + 1, sizeof(T));
            assert(buf);

            for (unsigned i = 0; i < size; ++i) {
                set(i, NEUTRAL);
            }

            return this;
        }

        FenwickTree *ctor(const T *src, unsigned new_size) {
            size = new_size;

            buf = (T *)calloc(size + 1, sizeof(T));
            assert(buf);

            for (unsigned i = 0; i < size; ++i) {
                set(i, src[i]);
            }

            return this;
        }

        void dtor() {
            free(buf);
            buf = nullptr;

            size = 0;
        }

        T request(unsigned left, unsigned right) const {
            return TRAITS::reverse(request(right), request(left - 1));
        }

        void update(unsigned ind, T delta) {
            for (; ind < size; ind |= ind + 1) {
                //printf("! buf[%u] (%lld) += %lld\n", ind, buf[ind], delta);
                buf[ind] = TRAITS::forward(buf[ind], delta);
            }
        }

        void set(unsigned ind, T value) {
            update(ind, TRAITS::reverse(value, request(ind, ind)));
        }

        void dump() {
            printf(">> ");

            for (unsigned i = 0; i < size; ++i)
                printf("%lld ", buf[i]);

            printf("\n");
        }

    private:

        T *buf;
        unsigned size;

        T request(unsigned ind) const {
            T result = NEUTRAL;

            for (; ind != (unsigned)-1; ind = (ind & (ind + 1)) - 1) {
                result = TRAITS::forward(result, buf[ind]);
            }

            return result;
        }

    };

}


/*

    We irreversibly compress one of the coordinates, then group up the segments by the other one, and then
    set up a Fenwick with the number of segments ending (or starting - we have inverted them) in a certain cell.
    Then we go and reset the Fenwick, as well as add the sum on the current prefix to the answer. That ensures
    that we only account for segments which haven't yes begun or have already ended.

*/

// Not sure if this can be called a functor anymore...
struct Traits_ {
    using T = long long;
    static const T neutral = 0;

    static constexpr T forward(T a, T b) {
        return a + b;
    }

    static constexpr T reverse(T a, T b) {
        return a - b;
    }
};


struct Segment {
    long long left;
    long long right;
    int count;

    bool operator<(const Segment &other) const {
        if (left != other.left)
            return left < other.left;

        return right > other.right;
    }

    bool operator==(const Segment &other) const {
        return left == other.left && right == other.right;
    }
};


int main() {
    typedef abel::FenwickTree<Traits_> Tree;
    typedef Traits_::T T;

    unsigned n = 0;
    int res = scanf("%u", &n);
    assert(res == 1);

    Segment *arr = (Segment *)calloc(n, sizeof(Segment));
    for (unsigned i = 0; i < n; ++i) {
        T x = 0, y = 0;
        res = scanf("%lld %lld", &x, &y);
        assert(res == 2);

        arr[i] = {x, y, 1};
    }

    // We compress the left coordinates, as well as swap every segment's coordinates to then
    // join them based based on the second coord. to implement the tricky repetition handling

    std::sort(arr, arr + n);

    T lastLeft = arr[0].left;
    T compressedLeft = 0;

    for (unsigned i = 0; i < n; ++i) {
        if (arr[i].left != lastLeft) {
            compressedLeft++;
            lastLeft = arr[i].left;
        }

        arr[i].left = arr[i].right;
        arr[i].right = compressedLeft;
    }

    std::sort(arr, arr + n);

    unsigned arrEnd = 0;

    for (unsigned i = 1; i < n; ++i) {
        if (arr[arrEnd] == arr[i]) {
            arr[arrEnd].count++;
        } else {
            arrEnd++;
            arr[arrEnd] = arr[i];
        }

    }
    arrEnd++;

    /*for (Segment *cur = arr; cur < arr + arrEnd; ++cur) {
        printf("[%lld, %lld] * %d\n", cur->left, cur->right, cur->count);
    }*/

    Tree tree{};
    tree.ctor(compressedLeft + 2);

    for (unsigned i = 0; i < arrEnd; ++i) {
        tree.update(arr[i].right, arr[i].count);
    }

    T ans = 0;

    for (unsigned i = 0; i < arrEnd; ++i) {
        tree.update(arr[i].right, -arr[i].count);  // We don't want to take this one into account

        ans += tree.request(0, arr[i].right) * arr[i].count;
    }

    tree.dtor();

    printf("%lld\n", ans);

    return 0;
}


/*

Complexity: O(n log n), because the most expensive thing we use is a Fenwick

*/

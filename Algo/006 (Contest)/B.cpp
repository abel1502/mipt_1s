/*

���������� ��������� ������ �� n ��������� a1,a2�an, �������������� ��������� ��������:

��������� �������� ai �������� j;
����� ����������������� ����� �� ������� �� l �� r ������������ (al\N{MINUS SIGN}al+1+al+2\N{MINUS SIGN}��ar).
������� ������
� ������ ������ �������� ����� ���������� ����������� ����� n (1\N{LESS-THAN OR EQUAL TO}n\N{LESS-THAN OR EQUAL TO}105) � ����� �������. �� ������ ������ �������� ��������� �������� ��������� (��������������� ����� �����, �� ������������� 104).

� ������� ������ ��������� ����������� ����� m (1\N{LESS-THAN OR EQUAL TO}m\N{LESS-THAN OR EQUAL TO}105) � ���������� ��������. � ����������� m ������� �������� ��������:

�������� ������� ���� �������� ����� ������� 0 i j (1\N{LESS-THAN OR EQUAL TO}i\N{LESS-THAN OR EQUAL TO}n, 1\N{LESS-THAN OR EQUAL TO}j\N{LESS-THAN OR EQUAL TO}104).
�������� ������� ���� �������� ����� ������� 1 l r (1\N{LESS-THAN OR EQUAL TO}l\N{LESS-THAN OR EQUAL TO}r\N{LESS-THAN OR EQUAL TO}n).
�������� ������
��� ������ �������� ������� ���� �������� �� ��������� ������ ��������������� ����������������� �����.

������
������� ������
3
1 2 3
5
1 1 2
1 1 3
1 2 3
0 2 1
1 1 3
�������� ������
-1
2
-1
3

*/

#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <cstring>


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

        FenwickTree *ctor(const T *src, unsigned new_size) {
            size = new_size;

            buf = (T *)calloc(size + 1, sizeof(T));

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

    We use a Fenwick's tree for sum on a subsegment, and achieve the funky sign behavior
    via manual inversion of every second element in the array (as well as new values for them)

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


int main() {
    typedef abel::FenwickTree<Traits_> Tree;
    typedef Traits_::T T;

    unsigned n = 0;
    int res = scanf("%u", &n);
    assert(res == 1);

    T *arr = (T *)calloc(n, sizeof(T));

    for (unsigned i = 0; i < n; ++i) {
        res = scanf("%lld", &arr[i]);
        assert(res == 1);

        if (i % 2)  arr[i] *= -1;
    }

    Tree tree{};
    tree.ctor(arr, n);

    unsigned m = 0;
    res = scanf("%u", &m);
    assert(res == 1);

    for (unsigned i = 0; i < m; ++i) {
        int op = -1;
        long long left = 0;
        long long right = 0;
        res = scanf("%d %lld %lld", &op, &left, &right);
        assert(res == 3);

        T result = 0;

        switch (op) {
        case 0:
            --left;

            if (left % 2)  right *= -1;

            tree.set(left, right);

            break;

        case 1:
            --left;
            --right;

            result = tree.request(left, right);
            if (left % 2)  result *= -1;

            printf("%lld\n", result);

            break;
        default:
            assert(false);
            abort();
        }
    }

    tree.dtor();

    return 0;
}


/*

Complexity: O(k log n), due to Fenwick

*/


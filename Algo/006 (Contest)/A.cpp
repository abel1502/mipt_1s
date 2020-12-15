/*

В начальный момент времени последовательность an задана следующей формулой: an=n2mod12345+n3mod23456. Требуется много раз отвечать на запросы следующего вида:

Найти разность между максимальным и минимальным значениями среди элементов ai,ai+1,…,aj.
Присвоить элементу ai значение j.
Входные данные
Первая строка входного файла содержит натуральное число k — количество запросов (1\N{LESS-THAN OR EQUAL TO}k\N{LESS-THAN OR EQUAL TO}100000). Следующие k строк содержат запросы, по одному на строке. Запрос номер i описывается двумя целыми числами xi, yi.

Если xi>0, то требуется найти разность между максимальным и минимальным значениями среди элементов axi,…,ayi. При этом 1\N{LESS-THAN OR EQUAL TO}xi\N{LESS-THAN OR EQUAL TO}yi\N{LESS-THAN OR EQUAL TO}100000.

Если xi<0, то требуется присвоить элементу a|xi| значение yi. В этом случае \N{MINUS SIGN}100000\N{LESS-THAN OR EQUAL TO}xi\N{LESS-THAN OR EQUAL TO}\N{MINUS SIGN}1 и |yi|\N{LESS-THAN OR EQUAL TO}100000.

Выходные данные
Для каждого запроса первого типа в выходной файл требуется вывести одну строку, содержащую разность между максимальным и минимальным значениями на соответствующем отрезке.

Пример
входные данные
7
1 3
2 4
-2 -100
1 5
8 9
-3 -101
2 3
выходные данные
34
68
250
234
1

*/

#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <cstring>


namespace abel {

    template <typename T, typename OP>
    class SegTree {
    public:

        static const T NEUTRAL;

        static SegTree *create() {
            SegTree *self = new SegTree();

            return self->ctor();
        }

        void destroy() {
            dtor();
            delete this;
        }

        SegTree *ctor() {
            buf = nullptr;
            size = 0;
            leavesStart = 0;

            return this;
        }

        SegTree *ctor(const T *src, unsigned new_size) {
            assert(!buf);
            assert(new_size < ((unsigned)-1)/2 - 10);

            size = new_size;

            unsigned bufSize = 1;
            while (bufSize <= size)  bufSize *= 2;
            bufSize *= 2;  // Not sure about this, but let's have it as a precaution

            buf = (T *)calloc(bufSize, sizeof(T));
            assert(buf);

            build(src, 0, 0, size);

            return this;
        }

        void dtor() {
            free(buf);
            buf = nullptr;

            size = 0;
            leavesStart = 0;
        }

        void build(const T *src, unsigned ind, unsigned left, unsigned right) {
            assert(left < right);

            if (left + 1 == right) {
                //printf("%lld\n", src[left].value);

                if (left == 0)  leavesStart = ind;

                if (left >= size)
                    buf[ind] = NEUTRAL;
                else
                    buf[ind] = src[left];

                return;
            }

            unsigned mid = (left + right) / 2;

            build(src, 2 * ind + 1, left, mid);
            build(src, 2 * ind + 2, mid, right);

            update(ind);
        }

        T request(unsigned reqLeft, unsigned reqRight) const {
            return request(0, 0, size, reqLeft, reqRight);
        }

        void set(unsigned pos, const T &value) {  // Maxim advised to pass it like this? and I can see his point
            set(0, 0, size, pos, value);
        }

        void dump() {
            dump(0, 0, size);
        }

    private:

        T *buf;
        unsigned size;
        unsigned leavesStart;

        T request(unsigned ind, unsigned left, unsigned right, unsigned reqLeft, unsigned reqRight) const {
            assert(left < right);

            if (reqLeft > reqRight || right <= reqLeft || left >= reqRight)
                return NEUTRAL;

            if (reqLeft <= left && right <= reqRight)
                return buf[ind];

            unsigned mid = (left + right) / 2;

            return OP()(
                request(2 * ind + 1, left, mid,  reqLeft, reqRight),
                request(2 * ind + 2, mid, right, reqLeft, reqRight)
            );
        }

        void set(unsigned ind, unsigned left, unsigned right, unsigned pos, const T &value) {
            assert(left < right);

            if (left + 1 == right) {
                assert(left == pos);

                buf[ind] = value;
                return;
            }

            unsigned mid = (left + right) / 2;

            if (pos < mid)
                set(2 * ind + 1, left, mid, pos, value);
            else
                set(2 * ind + 2, mid, right, pos, value);

            update(ind);
        }

        void update(unsigned ind) {
            buf[ind] = OP()(buf[2 * ind + 1], buf[2 * ind + 2]);
        }

        void dump(unsigned ind, unsigned left, unsigned right) {
            if (left > 100) {
                printf("...\n");
                return;
            }

            printf("%lld %lld ", buf[ind].min, buf[ind].max);

            if (left + 1 == right) {
                printf("\n");

                return;
            }

            printf("{\n");
            dump(2 * ind + 1, left, (left + right) / 2);
            printf(",\n");
            dump(2 * ind + 2, (left + right) / 2, right);
            printf("}\n");
        }

    };

}

/*

    Essentially, we maintain a segment tree with the minimum and maximum on subsegments.
    Then the required delta is just the result for the [l, r] segment. The only other
    thing to know is that my segment tree works on half-intervals, so to be more precise,
    the abovementioned request would be [l, r)

*/

struct Item {
    long long min;
    long long max;
};


struct OpFunctor {
    Item operator()(const Item &a, const Item &b) {
        return (Item){(a.min < b.min) ? a.min : b.min, (a.max > b.max) ? a.max : b.max};
    }
};


template <>
const Item abel::SegTree<Item, OpFunctor>::NEUTRAL = (Item){10000000, -10000000};


int main() {
    typedef abel::SegTree<Item, OpFunctor> Tree;

    const unsigned N = 100001;

    Item *arr = (Item *)calloc(N, sizeof(Item));

    for (long long i = 0; i < N; ++i) {
        long long x = (i*i) % 12345 + (i*i*i) % 23456;
        arr[i] = (Item){x, x};
    }

    Tree tree{};
    tree.ctor(arr, N);

    unsigned k = 0;
    int res = scanf("%u", &k);
    assert(res == 1);

    for (unsigned i = 0; i < k; ++i) {
        long long left = 0;
        long long right = 0;
        res = scanf("%lld %lld", &left, &right);
        assert(res == 2);

        assert(left != 0);

        if (left > 0) {
            Item result = tree.request(left, right + 1);

            printf("%lld\n", result.max - result.min);
        } else {
            tree.set(-left, (Item){right, right});
        }
    }

    tree.dtor();
    free(arr);

    return 0;
}

/*

Complexity: O(k log n), cause we're using a segment tree

*/


/*

Дано число N и последовательность из N целых чисел. Найти вторую порядковую статистику на заданных диапазонах.

Для решения задачи используйте структуру данных Sparse Table. Требуемое время обработки каждого диапазона O(1). Время подготовки структуры данных O(nlogn).

Входные данные
В первой строке заданы 2 числа: размер последовательности N и количество диапазонов M.

Следующие N целых чисел задают последовательность. Далее вводятся M пар чисел - границ диапазонов.

Выходные данные
Для каждого из M диапазонов напечатать элемент последовательности - 2ю порядковую статистику. По одному числу в строке.

Пример
входные данные
10 3
1 2 3 4 5 6 7 8 9 10
1 2
1 10
2 7
выходные данные
2
2
3
Примечание
Гарантируется, что каждый диапазон содержит как минимум 2 элемента.

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
        static const T POISON;

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
            overwrites = nullptr;
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

            overwrites = (T *)calloc(bufSize, sizeof(T));
            assert(overwrites);

            build(src, 0, 0, size);

            return this;
        }

        void dtor() {
            free(buf);
            buf = nullptr;

            free(overwrites);
            overwrites = nullptr;

            size = 0;
            leavesStart = 0;
        }

        void build(const T *src, unsigned ind, unsigned left, unsigned right) {
            assert(left < right);

            overwrites[ind] = POISON;

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

        T request(unsigned reqLeft, unsigned reqRight) {
            return request(0, 0, size, reqLeft, reqRight);
        }

        void set(unsigned pos, const T &value) {
            set(0, 0, size, pos, value);
        }

        void setSegment(unsigned reqLeft, unsigned reqRight, const T &value) {
            setSegment(0, 0, size, reqLeft, reqRight, value);
        }

        void dump() {
            dump(0, 0, size);
        }

    private:

        T *buf;
        T *overwrites;
        unsigned size;
        unsigned leavesStart;

        T request(unsigned ind, unsigned left, unsigned right, unsigned reqLeft, unsigned reqRight) {
            assert(left < right);

            if (reqLeft > reqRight || right <= reqLeft || left >= reqRight)
                return NEUTRAL;

            if (overwrites[ind] != POISON) {
                if (left + 1 == right) {
                    buf[ind] = overwrites[ind];
                    overwrites[ind] = POISON;
                } else {
                    push(ind);
                    update(ind);
                }
            }

            //printf(">>> %2u:[%u..%u) - %d\n", ind, left, right, buf[ind].min);

            if (reqLeft <= left && right <= reqRight)
                return buf[ind];

            unsigned mid = (left + right) / 2;

            push(ind);
            update(ind);

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
                overwrites[ind] = POISON;
                return;
            }

            push(ind);
            update(ind);

            unsigned mid = (left + right) / 2;

            if (pos < mid)
                set(2 * ind + 1, left, mid,  pos, value);
            else
                set(2 * ind + 2, mid, right, pos, value);

            update(ind);
        }

        void setSegment(unsigned ind, unsigned left, unsigned right, unsigned reqLeft, unsigned reqRight, const T &value) {
            assert(left < right);

            if (reqLeft > reqRight || right <= reqLeft || left >= reqRight)
                return;

            if (left + 1 < right) {
                push(ind);
                update(ind);
            }

            if (reqLeft <= left && right <= reqRight) {
                if (left + 1 < right) {
                    overwrites[ind] = value;
                    push(ind);
                    update(ind);
                } else {
                    buf[ind] = value;
                    overwrites[ind] = POISON;
                }

                return;
            }

            unsigned mid = (left + right) / 2;

            setSegment(2 * ind + 1, left, mid,  reqLeft, reqRight, value);
            setSegment(2 * ind + 2, mid, right, reqLeft, reqRight, value);

            if (left + 1 < right)
                update(ind);
        }

        void update(unsigned ind) {
            T left = buf[2 * ind + 1];
            T right = buf[2 * ind + 2];

            if (overwrites[2 * ind + 1] != POISON)
                left = overwrites[2 * ind + 1];
            if (overwrites[2 * ind + 2] != POISON)
                right = overwrites[2 * ind + 2];

            buf[ind] = OP()(left, right);
        }

        void push(unsigned ind) {
            if (overwrites[ind] == POISON)
                return;

            overwrites[2 * ind + 1] = overwrites[ind];
            overwrites[2 * ind + 2] = overwrites[ind];
            overwrites[ind] = POISON;
        }

        void dump(unsigned ind, unsigned left, unsigned right) {
            if (left > 100) {
                printf("...\n");
                return;
            }

            printf("%d %d (%d %d)", buf[ind].min, buf[ind].max, overwrites[ind].min, overwrites[ind].max);

            if (left + 1 == right) {
                printf(" <");

                return;
            }

            printf(" {\n");
            dump(2 * ind + 1, left, (left + right) / 2);
            printf(",\n");
            dump(2 * ind + 2, (left + right) / 2, right);
            printf("\n}");
        }

    };

}

/*

    I already have a min-max segment tree, why not just copy it? I'm even too lazy to remove the maximum part.
    (To be honest, it's not due to laziness, but because I'm expecting to reuse this code later)

*/

struct Item {
    int min;
    int max;

    bool operator==(const Item &other) const {
        return min == other.min && max == other.max;
    }

    bool operator!=(const Item &other) const {
        return min != other.min || max != other.max;
    }
};


struct OpFunctor {
    Item operator()(const Item &a, const Item &b) const {
        return (Item){(a.min < b.min) ? a.min : b.min, (a.max > b.max) ? a.max : b.max};
    }
};


template <>
const Item abel::SegTree<Item, OpFunctor>::NEUTRAL = (Item){10000000, -10000000};

template <>
const Item abel::SegTree<Item, OpFunctor>::POISON = (Item){-1, -1};


int main() {
    typedef abel::SegTree<Item, OpFunctor> Tree;

    unsigned n = 0;
    int res = scanf("%u", &n);
    assert(res == 1);

    Item *arr = (Item *)calloc(n, sizeof(Item));

    for (unsigned i = 0; i < n; ++i) {
        int r = 0, g = 0, b = 0;
        res = scanf("%d %d %d", &r, &g, &b);
        assert(res == 3);

        arr[i] = (Item){r + g + b, r + g + b};
    }

    Tree tree{};
    tree.ctor(arr, n);

    unsigned k = 0;
    res = scanf("%u", &k);
    assert(res == 1);

    for (unsigned i = 0; i < k; ++i) {
        //tree.dump();printf("\n");

        unsigned leftPaint = 0, rightPaint = 0, leftReq = 0, rightReq = 0;
        int r = 0, g = 0, b = 0;
        res = scanf("%u %u %d %d %d %u %u", &leftPaint, &rightPaint, &r, &g, &b, &leftReq, &rightReq);
        assert(res == 7);

        tree.setSegment(leftPaint, rightPaint + 1, (Item){r + g + b, r + g + b});

        // TODO: space-separate
        printf("%d ", tree.request(leftReq, rightReq + 1).min);
    }
    printf("\n");

    //tree.dump();printf("\n");

    tree.dtor();
    free(arr);

    return 0;
}

/*

Complexity: O(k log n), since we're using a segment tree (same as in A)

*/



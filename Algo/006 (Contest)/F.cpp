//#define ALGO_DEBUG

/*

¬ас€ любит наблюдать за звездами. Ќо следить за всем небом сразу ему т€жело. ѕоэтому он
наблюдает только за частью пространства, ограниченной кубом размером n\N{MULTIPLICATION SIGN}n\N{MULTIPLICATION SIGN}n. Ётот куб поделен
на маленькие кубики размером 1 \N{MULTIPLICATION SIGN} 1 \N{MULTIPLICATION SIGN} 1. ¬о врем€ его наблюдений могут происходить следующие
событи€:
1. ¬ каком-то кубике по€вл€ютс€ или исчезают несколько звезд.
2.   нему может загл€нуть его друг ѕет€ и поинтересоватьс€, сколько видно звезд в части пространства, состо€щей из нескольких кубиков.
‘ормат входных данных
ѕерва€ строка входного файла содержит натуральное число 1 6 n 6 128.  оординаты кубиков
Ч целые числа от 0 до n \N{MINUS SIGN} 1. ƒалее следуют записи о происходивших событи€х по одной в строке.
¬ начале строки записано число m. ≈сли m равно:
1, то за ним следуют 4 числа Ч x, y, z (0 6 x, y, z < N) и k (\N{MINUS SIGN}20000 6 k 6 20000) Ч координаты
кубика и величина, на которую в нем изменилось количество видимых звезд;
2, то за ним следуют 6 чисел Ч x1, y1, z1, x2, y2, z2 (0 6 x1 6 x2 < N, 0 6 y1 6 y2 < N,
0 6 z1 6 z2 < N), которые означают, что ѕет€ попросил подсчитать количество звезд в
кубиках (x, y, z) из области: x1 6 x 6 x2, y1 6 y 6 y2, z1 6 z 6 z2;
3, то это означает, что ¬асе надоело наблюдать за звездами и отвечать на вопросы ѕети. Ёта
запись встречаетс€ во входном файле только один раз и будет последней записью.
 оличество записей во входном файле не больше 100 002.
‘ормат выходных данных
ƒл€ каждого ѕетиного вопроса выведите на отдельной строке одно число Ч искомое количество
звезд.
ѕример
стандартный ввод стандартный вывод
2
2 1 1 1 1 1 1
1 0 0 0 1
1 0 1 0 3
2 0 0 0 0 0 0
2 0 0 0 0 1 0
1 0 1 0 -2
2 0 0 0 1 1 1
3
0
1
4
2

*/


#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <cstring>


namespace abel {

    // Since it's now cubic, I presumed

    // OP must provide static methods ::forward, ::reverse, such that
    // for any x, y reverse(forward(x, y), y) == x, as well as
    // typename ::T and static const T ::neutral, which is, well,
    // neutral to ::forward and ::backward
    template <typename TRAITS>
    class FenwickCubicTree {
    public:

        using T = typename TRAITS::T;
        static const T NEUTRAL = TRAITS::neutral;

        static FenwickCubicTree *create() {
            FenwickCubicTree *self = new FenwickCubicTree();

            return self->ctor();
        }

        void destroy() {
            dtor();
            delete this;
        }

        FenwickCubicTree *ctor() {
            buf = nullptr;
            size = 0;

            return this;
        }

        FenwickCubicTree *ctor(int new_size) {
            size = new_size;

            buf = (T *)calloc(size * size * size + 1, sizeof(T));
            assert(buf);

            return this;
        }

        void dtor() {
            free(buf);
            buf = nullptr;

            size = 0;
        }

        T request(int leftX, int leftY, int leftZ, int rightX, int rightY, int rightZ) const {
            leftX--;
            leftY--;
            leftZ--;

            T c222 = request(rightX, rightY, rightZ);

            T c122 = request(leftX, rightY, rightZ);
            T c212 = request(rightX, leftY, rightZ);
            T c221 = request(rightX, rightY, leftZ);

            T c112 = request(leftX, leftY, rightZ);
            T c211 = request(rightX, leftY, leftZ);
            T c121 = request(leftX, rightY, leftZ);

            T c111 = request(leftX, leftY, leftZ);

            //printf(">> %lld  %lld %lld %lld  %lld %lld %lld  %lld\n", c222, c122, c212, c221, c112, c211, c121, c111);

            #define ADD_(A, B)  TRAITS::forward(A, B)
            #define SUB_(A, B)  TRAITS::reverse(A, B)

            // c222 - c221 - c212 -c122 + c112 + c121 + c211 - c111
            return SUB_(ADD_(SUB_(SUB_(SUB_(c222, c122), c212), c221),
                             ADD_(ADD_(c112, c211), c121)),
                        c111);

            #undef ADD_
            #undef SUB_
        }

        void update(int x0, int y0, int z0, T delta) {
            for (int x = x0; x < size; x |= x + 1)
                for (int y = y0; y < size; y |= y + 1)
                    for (int z = z0; z < size; z |= z + 1)
                        get(x, y, z) = TRAITS::forward(get(x, y, z), delta);
        }

        void dump() {
            printf("Fenwick {");

            for (int x = 0; x < size; ++x) {
                for (int y = 0; y < size; ++y) {
                    printf("\n    ");

                    for (int z = 0; z < size; ++z) {
                        printf("%2lld ", get(x, y, z));
                        //printf("%2lld ", request(x, y, z, x, y, z));
                    }
                }

                printf("\n");
            }

            printf("}\n");
        }

    private:

        T *buf;
        int size;

        T request(int x0, int y0, int z0) const {
            T result = NEUTRAL;

            for (int x = x0; x >= 0; x = (x & (x + 1)) - 1)
                for (int y = y0; y >= 0; y = (y & (y + 1)) - 1)
                    for (int z = z0; z >= 0; z = (z & (z + 1)) - 1)
                        result = TRAITS::forward(result, get(x, y, z));

            return result;
        }

        // Just like with left() and right() in my AVL, this is a convenience wrapper
        // (except now I know to prefix it with constexpr instead of inline)
        constexpr T &get(int x, int y, int z) const {
            return buf[size * size * x + size * y + z];
        }

    };

}


/*

    The task was to make a three-dimensional Fenwick, and I did.
    Don't think there's much more to explain.

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

    bool operator<(const Segment &other) {
        if (left != other.left)
            return left < other.left;

        return right > other.right;
    }

    bool operator==(const Segment &other) {
        return left == other.left && right == other.right;
    }
};


int main() {
    typedef abel::FenwickCubicTree<Traits_> Tree;
    typedef Traits_::T T;

    int n = 0;
    int res = scanf("%d", &n);
    assert(res == 1);

    Tree tree{};
    tree.ctor(n);

    bool repeat = true;

    while (repeat) {
        //tree.dump();

        int op = 3;
        res = scanf("%d", &op);
        assert(res == 1);

        switch (op) {
        case 1: {
            int x = 0, y = 0, z = 0;
            T k = 0;
            res = scanf("%d %d %d %lld", &x, &y, &z, &k);
            assert(res == 4);

            tree.update(x, y, z, k);
        } break;
        case 2: {
            int x1 = 0, y1 = 0, z1 = 0;
            int x2 = 0, y2 = 0, z2 = 0;

            res = scanf("%d %d %d %d %d %d", &x1, &y1, &z1, &x2, &y2, &z2);
            assert(res == 6);

			printf("%lld\n", tree.request(x1, y1, z1, x2, y2, z2));
        } break;
        case 3: {
            repeat = false;
        } break;
        default:
            assert(false);
            abort();
        }
    }

    tree.dtor();

    return 0;
}


/*

Complexity: O(n^3 + (log n)^3 * m), where m is the number of requests (which is thankfully constant),
            where log^3 comes from a cubic Fenwick, ad n^3 - is the allocation price.

*/

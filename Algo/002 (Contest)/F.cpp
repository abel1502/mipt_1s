#define ALGO_DEBUG

#include <cstdio>
#include <cstdlib>
#include <cstring>

#ifdef ALGO_DEBUG
#include <cassert>
#else
#define assert(stmt)  if (!(stmt)) exit(__LINE__);
#endif


template <typename T, typename F>
class SegTree {
public:
    static const T neutral;

    static SegTree<T, F> *create() {
        SegTree<T, F> *self = new SegTree<T, F>();
        assert(self);

        return self->ctor();
    }

    SegTree<T, F> *ctor() {
        size = 0;
        capacity = 0;
        buf = nullptr;

        return this;
    }

    SegTree<T, F> *ctor(unsigned new_size) {
        size = new_size;
        capacity = 1;

        while (capacity < size * 2) {
            capacity *= 2;
        }

        buf = (T *)calloc(2 * capacity, sizeof(T));
        assert(buf);

        for (unsigned i = 0; i < 2 * capacity; ++i) {
            buf[i] = neutral;
        }

        return this;
    }

    SegTree<T, F> *ctor(unsigned new_size, const T *source) {
        ctor(new_size);

        build(source);

        return this;
    }

    void dtor() {
        if (buf) {
            free(buf);
        }
    }

    void build(const T *source) {
        build(source, 1, 0, capacity - 1);
    }

    T get(unsigned leftReq, unsigned rightReq) {
        return get(1, leftReq, rightReq, 0, capacity - 1);
    }

    void update(unsigned ind, T value) {
        /*for (unsigned i = 0; i < 4 * size; ++i) {
            if (i == capacity) printf(">");
            printf("%d ", buf[i]);
        }
        printf("\n");*/

        ind += capacity;

        buf[ind] = value;

        ind /= 2;

        while (ind) {
            buf[ind] = F()(buf[2 * ind], buf[2 * ind + 1]);

            //printf("[%u] = %d\n", ind, buf[ind]);

            ind /= 2;
        }
    }

protected:
    unsigned size;
    unsigned capacity;
    T *buf;

    void build(const T *source, unsigned curNode, unsigned leftEdge, unsigned rightEdge) {
        if (leftEdge > rightEdge)  return;

        if (leftEdge == rightEdge) {
            buf[curNode] = source[leftEdge];

            return;
        }

        unsigned midEdge = (leftEdge + rightEdge) / 2;
        build(source, curNode * 2, leftEdge, midEdge);
        build(source, curNode * 2 + 1, midEdge + 1, rightEdge);
        buf[curNode] = F()(buf[curNode * 2], buf[curNode * 2 + 1]);
    }

    T get(unsigned curNode, unsigned leftReq, unsigned rightReq, unsigned leftEdge, unsigned rightEdge) {
        //printf("> %u %u %u %u %u\n", curNode, leftReq, rightReq, leftEdge, rightEdge);

        if (leftReq > rightReq) {
            return neutral;
        }

        if (leftReq == leftEdge && rightReq == rightEdge) {
            return buf[curNode];
        }

        unsigned midEdge = (leftEdge + rightEdge) / 2;

        return F()(
            get(curNode * 2, leftReq, rightReq > midEdge ? midEdge : rightReq, leftEdge, midEdge),
            get(curNode * 2 + 1, leftReq < midEdge + 1 ? midEdge + 1 : leftReq, rightReq, midEdge + 1, rightEdge)
        );
    }

};


struct XorIntFunctor {
    int operator()(int a, int b) {
        return a ^ b;
    }
};


template<>
const int SegTree<int, XorIntFunctor>::neutral = 0;


int main() {
    int res = 0;

    unsigned n = 0;
    unsigned m = 0;

    res = scanf("%u %u", &n, &m);
    assert(res == 2);

    int *v = (int *)calloc(n, sizeof(int));
    assert(v);

    for (unsigned i = 0; i < n; ++i) {
        res = scanf("%d", &v[i]);
        assert(res == 1);
    }

    SegTree<int, XorIntFunctor> *tree = SegTree<int, XorIntFunctor>::create()->ctor(n, v);

    for (unsigned i = 0; i < m; ++i) {
        int code = 0;

        res = scanf("%d", &code);
        assert(res == 1);

        switch (code) {
        case 1: {
            unsigned left = 0, right = 0;

            res = scanf("%u %u", &left, &right);
            assert(res == 2);

            printf("%d\n", tree->get(left, right));
        } break;
        case 2: {
            unsigned ind = 0;
            int val = 0;

            res = scanf("%u %d", &ind, &val);
            assert(res == 2);

            tree->update(ind, val);
        } break;
        default:
            assert(false);
            return 1;
        }
    }

    tree->dtor();

    return 0;
}

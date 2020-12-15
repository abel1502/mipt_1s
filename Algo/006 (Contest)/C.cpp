//#define ALGO_DEBUG

/*

Дан массив a из n чисел. Нужно обрабатывать запросы:
0. set(i, x) – a[i] = x;
1. get(i, x) – найти min k : k > i и ak > x.
Формат входных данных
На первой строке длина массива n и количество запросов m. На второй строке n целых чисел –
массив a. Следующие m строк содержат запросы.
Индексы в массиве нумеруются с 1.
Запрос типа set: “0 i x”.
Запрос типа get: “1 i x”.
1 6 n, m 6 200 000.
1 6 i 6 n.
0 6 x, ai 6 200 000.
Формат выходных данных
На каждой запрос типа get на отдельной строке выведите k.
Если такого k не существует, выведите \N{MINUS SIGN}1.
Пример
стандартный ввод стандартный вывод
4 5
1 2 3 4
1 1 1
1 1 3
1 1 5
0 2 3
1 1 3
1
3
-1
2

*/

#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <cstring>

#ifdef ALGO_DEBUG
#include <string>  // For to_string for <T> dump
#endif


namespace abel {

    // Remember this mess? I copied it from the previous contest)

    template <typename T> class Treap;  // Forward declaration for befriending


    template <typename T>
    class TreapNode {
        friend class Treap<T>;
    public:
        T key;
        T stMax;  // subtree max (can't just name it max due to there being a method of this name)

        static const T INF;

        static TreapNode<T> *const STUB;

        static TreapNode<T> *create() {
            TreapNode<T> *self = new TreapNode<T>();
            return self->ctor();
        }

        void destroy() {
            dtor();
            delete this;
        }

        TreapNode<T> *ctor() {
            key = {};
            stMax = -INF;
            size = 0;
            // I guess we want to make sure that STUB elements are also leaves by the heap criteria...
            priority = (unsigned long long)-1;
            children[0] = nullptr;
            children[1] = nullptr;

            return this;
        }

        TreapNode<T> *ctor(T new_key) {
            key = new_key;
            stMax = new_key;
            size = 1;
            priority = (rand() << 16) | rand();
            children[0] = STUB;
            children[1] = STUB;

            return this;
        }

        TreapNode<T> *ctor(T new_key, TreapNode<T> *new_left, TreapNode<T> *new_right) {
            key = new_key;
            stMax = -INF;
            size = 0;
            priority = (rand() << 16) | rand();
            children[0] = new_left;
            children[1] = new_right;

            fix();

            return this;
        }

        void dtor() {
            for (int i = 0; i < 2; ++i) {
                if (!children[i] || children[i]->isStub())  continue;
                children[i]->dtor();
                free(children[i]);
            }
        }

        inline bool isStub() {
            return size == 0;
        }

        inline unsigned getSize() {
            return size;
        }

        TreapNode<T> *edgeChild(bool isRight) {
            assert(!isStub());

            if (!children[isRight]->isStub())
                return children[isRight]->edgeChild(isRight);

            return this;
        }

        // Convenience wrappers
        TreapNode<T> *min() {
            return edgeChild(false);
        }

        TreapNode<T> *max() {
            return edgeChild(true);
        }

        TreapNode<T> *erase() {
            if (isStub()) return STUB;

            TreapNode<T> *lNode = left();
            TreapNode<T> *rNode = right();
            left() = STUB;
            right() = STUB;

            delete this;

            return merge({lNode, rNode});
        }

        TreapNode<T> *eraseKey(T goal_key) {
            if (isStub()) return STUB;

            if (key == goal_key) return erase();

            bool isRight = goal_key > key;

            children[isRight] = children[isRight]->eraseKey(goal_key);

            fix();

            return this;
        }

        TreapNode<T> *eraseInd(unsigned ind) {
            if (isStub()) return STUB;

            unsigned lSize = left()->size;

            if (lSize == ind) {
                return erase();
            } else if (lSize > ind) {
                left() = left()->eraseInd(ind);
            } else {
                right() = right()->eraseInd(ind - lSize - 1);
            }

            fix();

            return this;
        }

        TreapNode<T> *insertKey(T new_key) {
            if (isStub()) return TreapNode<T>::create()->ctor(new_key);

            if (!findKey(new_key)->isStub()) return this;

            TreapNode<T> *newNode = TreapNode<T>::create()->ctor(new_key);

            TreapNode<T> *rightSubtree = splitKey(new_key);

            return merge({merge({this, newNode}), rightSubtree});
        }

        TreapNode<T> *insertInd(unsigned ind, T new_key) {
            if (isStub()) return TreapNode<T>::create()->ctor(new_key);

            TreapNode<T> *newNode = TreapNode<T>::create()->ctor(new_key);

            Pair result = splitInd(ind);

            return merge({merge({result.left, newNode}), result.right});
        }

        TreapNode<T> *findKey(T goal_key) {
            if (isStub()) return STUB;

            if (key == goal_key) return this;

            return children[goal_key > key]->find(goal_key);
        }

        TreapNode<T> *findInd(unsigned ind) {
            if (isStub()) return STUB;

            unsigned lSize = left()->size;

            if (lSize == ind) {
                return this;
            } else if (lSize > ind) {
                return left()->findInd(ind);
            } else {
                return right()->findInd(ind - lSize - 1);
            }
        }

        TreapNode<T> *findClosest(T goal_key, bool isRight) {
            /*printf("> %p ", this);
            if (this) printf("(key=%d)", key);
            printf("\n");*/

            if (isStub()) return STUB;

            if (key == goal_key) {  // Maybe an unnecessary optimization, but why not
                return children[isRight]->edgeChild(!isRight);
            }

            TreapNode<T> *result = children[goal_key > key]->findClosest(goal_key, isRight);

            if (result->isStub() && (isRight == (goal_key < key))) return this;  // This essentially says that if the goal subtree is empty, the current node is the goal

            return result;
        }

        TreapNode<T> *minAbove(T goal_key) {
            return findClosest(goal_key, true);
        }

        TreapNode<T> *maxBelow(T goal_key) {
            return findClosest(goal_key, false);
        }

        T findLeftmostAbove(T goal_key) {
            if (isStub())
                return -1;

            if (left()->stMax >= goal_key)
                return left()->findLeftmostAbove(goal_key);

            if (key >= goal_key)
                return (T)left()->size;

            if (right()->stMax >= goal_key)
                return (T)left()->size + 1 + right()->findLeftmostAbove(goal_key);

            return -1;
        }

        struct Pair {
            TreapNode<T> *left;
            TreapNode<T> *right;
        };

        Pair splitKey(T goal_key) {
            if (isStub()) {
                return {STUB, STUB};
            }

            if (goal_key > key) {
                Pair result = right()->splitKey(goal_key);

                right() = result.left;

                fix();

                return {this, result.right};
            } else {
                Pair result = left()->splitKey(goal_key);

                left() = result.right;

                fix();

                return {result.left, this};
            }
        }

        Pair splitInd(unsigned ind) {
            if (isStub()) {
                return {STUB, STUB};
            }

            unsigned lSize = left()->size;

            if (ind > lSize) {
                Pair result = right()->splitInd(ind - lSize - 1);

                right() = result.left;

                fix();

                return {this, result.right};
            } else {
                Pair result = left()->splitInd(ind);

                left() = result.right;

                fix();

                return {result.left, this};
            }
        }

        static TreapNode<T> *merge(Pair trees) {
            if (trees.left->isStub()) return trees.right;

            if (trees.right->isStub()) return trees.left;

            if (trees.left->priority > trees.right->priority) {
                trees.left->right() = merge({trees.left->right(), trees.right});

                trees.left->fix();

                return trees.left;
            } else {
                trees.right->left() = merge({trees.left, trees.right->left()});

                trees.right->fix();

                return trees.right;
            }
        }

        // I'm not sure if operator[] would work as intended for setting values, so I'll do it this way

        T get(unsigned ind) {
            TreapNode<T> *goal = findInd(ind);

            assert(!goal->isStub());

            return goal->key;
        }

        void set(unsigned ind, T val) {
            assert(!isStub());

            unsigned lSize = left()->size;

            if (lSize == ind) {
                key = val;
            } else if (lSize > ind) {
                left()->set(ind, val);
            } else {
                right()->set(ind - lSize - 1, val);
            }

            fix();
        }

        #ifdef ALGO_DEBUG
        void dump(int depth = 0) {
            for (int i = 0; i < depth; ++i) {
                printf("  ");
            }
            printf("[%p]", this);
            if (isStub()) {
                printf(" (STUB)\n");
            } else {
                //printf(" (%s):\n", std::to_string(key).c_str());
                printf(" (%s, %u, %s):\n", std::to_string(key).c_str(), size, std::to_string(stMax).c_str());

                left ()->dump(depth + 1);
                right()->dump(depth + 1);
            }
        }

        void dumpLinear() {
            if (isStub()) return;

            left ()->dumpLinear();
            printf("%s ", std::to_string(key).c_str());
            right()->dumpLinear();
        }
        #endif // ALGO_DEBUG

    #ifndef ALGO_DEBUG
    protected:
    #endif
        unsigned long long priority;
        TreapNode<T> *children[2];  // {left, right}
        unsigned size;

        static TreapNode<T> STUBVAL;

        // The following two methods are provided solely for convenience purposes as aliases
        // for children[0] and children[1]. The same effect could have been achieved with
        // #define left children[0] and #define right children[1], but I decided
        // this approach to be a better code practice
        inline TreapNode<T> *&left() {
            assert(!isStub());

            return children[0];
        }

        inline TreapNode<T> *&right() {
            assert(!isStub());

            return children[1];
        }

        void fix() {
            if (isStub()) return;

            size = 1 + left()->size + right()->size;

            stMax = key;

            if (left()->stMax > stMax) {
                stMax = left()->stMax;
            }

            if (right()->stMax > stMax) {
                stMax = right()->stMax;
            }
        }
    };


    template <typename T>
    class Treap {
    public:
        static Treap<T> *create() {
            Treap<T> *self = new Treap<T>();

            return self->ctor();
        }

        void destroy() {
            dtor();
            delete this;
        }

        Treap<T> *ctor() {
            root = TreapNode<T>::STUB;

            return this;
        }

        void dtor() {
            if (root && !root->isStub()) {
                root->dtor();
                free(root);
            }
        }

        TreapNode<T> *getRootSubtree() {
            return root;
        }

        void insertKey(T key) {
            root = root->insertKey(key);
        }

        void insertInd(unsigned ind, T key) {
            root = root->insertInd(ind, key);
        }

        void insertBack(T key) {
            root = root->insertInd(getSize() + 1, key);
        }

        void insertFront(T key) {
            root = root->insertInd(0, key);
        }

        void eraseKey(T key) {
            root = root->eraseKey(key);
        }

        void eraseInd(unsigned ind) {
            root = root->eraseInd(ind);
        }

        bool isEmpty() {
            return root->isStub();
        }

        unsigned getSize() {
            return root->getSize();
        }

        TreapNode<T> *findKey(T key) {
            return root->findKey(key);
        }

        TreapNode<T> *findInd(unsigned ind) {
            return root->findInd(ind);
        }

        bool contains(T key) {
            return !findKey(key)->isStub();
        }

        T max() {
            assert(!isEmpty());

            return root->max()->key;
        }

        T min() {
            assert(!isEmpty());

            return root->min()->key;
        }

        // Since we don't know if T has a poisonous value, we'll have to return key in
        // the following two functions in case of absence of the requested element
        T maxBelow(T key) {
            if (root->isStub()) {
                return key;
            }

            TreapNode<T> *result = root->maxBelow(key);

            return !result->isStub() ? result->key : key;
        }

        T minAbove(T key) {
            if (root->isStub()) {
                return key;
            }

            TreapNode<T> *result = root->minAbove(key);

            return !result->isStub() ? result->key : key;
        }

        T get(unsigned ind) {
            return root->get(ind);
        }

        void set(unsigned ind, T val) {
            root->set(ind, val);
        }

        T handleClosestLeftAbove(unsigned ind, T value) {
            //if (ind >= getSize())
            //    return -1;

            struct TreapNode<T>::Pair splitted = root->splitInd(ind);

            T result = splitted.right->findLeftmostAbove(value);

            if (result >= 0) {
                result += splitted.left->getSize() + 1;
            }

            root = TreapNode<T>::merge({splitted.left, splitted.right});

            if (result < 0 && get(ind) >= value)
                result = ind + 1;

            return result;
        }

        #ifdef ALGO_DEBUG
        void dump() {
            printf("Treap [%p] {\n", this);
            root->dump(1);
            printf("}\n");
        }

        void dumpLinear() {
            printf("Treap: ");
            root->dumpLinear();
            printf("\n");
        }
        #endif // ALGO_DEBUG

    #ifndef ALGO_DEBUG
    protected:
    #endif
        TreapNode<T> *root;
    };

}

template<>
const long long abel::TreapNode<long long>::INF = 1000000000;

template <typename T>
abel::TreapNode<T> abel::TreapNode<T>::STUBVAL{};

template <typename T>
abel::TreapNode<T> *const abel::TreapNode<T>::STUB = &abel::TreapNode<T>::STUBVAL;

/*
    I looked through some discussions on this task in our vkontakte chat, and saw several people suggest
    to use a treap for it instead of segment trees. I guess they're right - so here we are.
    We use an implicit key treap wich also maintains the maximal element on a subsegment. Then we may
    split the treap by i and find the desired element in the right subtree by descending it in such a way
    that we go as left as we can but watch the maximum to be above or equal to x.
*/

int main() {
    typedef long long T;
    typedef abel::Treap<T> Tree;

    Tree tree{};
    tree.ctor();

    unsigned n = 0;
    unsigned m = 0;
    int res = scanf("%u %u", &n, &m);
    assert(res == 2);

    for (unsigned i = 0; i < n; ++i) {
        T tmp = 0;
        res = scanf("%lld", &tmp);
        assert(res == 1);

        tree.insertBack(tmp);
        tree.set(tree.getSize() - 1, tmp);
    }

    for (unsigned i = 0; i < m; ++i) {
        #ifdef ALGO_DEBUG
        tree.dump();
        #endif // ALGO_DEBUG

        int op = -1;
        unsigned ind = 0;
        T val = 0;
        res = scanf("%d %u %lld", &op, &ind, &val);
        assert(res == 3);

        switch (op) {
        case 0:
            tree.set(ind - 1, val);

            break;
        case 1:
            printf("%lld\n", tree.handleClosestLeftAbove(ind - 1, val));

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

Complexity: O(m log n). Sometimes I wonder if I really need to tell you the common trees' asymptotics every time)...

*/



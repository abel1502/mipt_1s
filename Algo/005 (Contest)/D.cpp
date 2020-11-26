//#define ALGO_DEBUG

/*

Во Флатландии протекает богатая рыбой река Большой Флат. Много лет назад река была поделена между n рыболовными предприятиями, каждое из которых получило непрерывный отрезок реки. При этом i-е предприятие, если рассматривать их по порядку, начиная от истока, изначально получило отрезок реки длиной ai.

С тех пор с рыболовными предприятиями во Флатландии k раз происходили различные события. Каждое из событий было одного из двух типов: банкротство некоторого предприятия или разделение некоторого предприятия на два. При некоторых событиях отрезок реки, принадлежащий предприятию, с которым это событие происходит, делится на две части. Каждый такой отрезок имеет длину большую или равную 2. Деление происходит по следующему правилу. Если отрезок имеет четную длину, то он делится на две равные части. Иначе он делится на две части, длины которых различаются ровно на единицу, при этом часть, которая ближе к истоку реки, имеет меньшую длину.

При банкротстве предприятия происходит следующее. Отрезок реки, принадлежавший обанкротившемуся предприятию, переходит к его соседям. Если у обанкротившегося предприятия один сосед, то этому соседу целиком передается отрезок реки обанкротившегося предприятия. Если же соседей двое, то отрезок реки делится на две части описанным выше способом, после чего каждый из соседей присоединяет к своему отрезку ближайшую к нему часть. При разделении предприятия отрезок реки, принадлежавший разделяемому предприятию, всегда делится на две части описанным выше способом. Разделившееся предприятие ликвидируется, и образуются два новых предприятия. Таким образом, после каждого события каждое предприятие владеет некоторым отрезком реки.

Министерство финансов Флатландии предлагает ввести налог на рыболовные предприятия, пропорциональный квадрату длины отрезка реки, принадлежащего соответствующему предприятию. Чтобы проанализировать, как будет работать этот налог, министр хочет по имеющимся данным узнать, как изменялась величина, равная сумме квадратов длин отрезков реки, принадлежащих предприятиям, после каждого произошедшего события.

Требуется написать программу, которая по заданному начальному разделению реки между предприятиями и списку событий, происходивших с предприятиями, определит, чему равна сумма квадратов длин отрезков реки, принадлежащих предприятиям, в начальный момент времени и после каждого события.

Входные данные
Первая строка входного файла содержит два целых числа: n и p — исходное количество предприятий (2\N{LESS-THAN OR SLANTED EQUAL TO}n\N{LESS-THAN OR SLANTED EQUAL TO}100000) и номер подзадачи (0\N{LESS-THAN OR SLANTED EQUAL TO}p\N{LESS-THAN OR SLANTED EQUAL TO}4) (считайте его просто так).

Вторая строка входного файла содержит n целых чисел a1, a2, …, an — длины исходных отрезков реки.

Третья строка входного файла содержит целое число k — количество событий, происходивших с предприятиями (1\N{LESS-THAN OR SLANTED EQUAL TO}k\N{LESS-THAN OR SLANTED EQUAL TO}100000).

Последующие k строк содержат описания событий, i-я строка содержит два целых числа: ei и vi — тип события и номер предприятия, с которым оно произошло. Значение ei = 1 означает, что предприятие, которое после всех предыдущих событий является vi-м по порядку, если считать с единицы от истока реки, обанкротилось, а значение ei = 2 означает, что это предприятие разделилось на два.

Гарантируется, что значение vi не превышает текущее количество предприятий. Гарантируется, что если отрезок предприятия при банкротстве или разделении требуется поделить на две части, то он имеет длину большую или равную 2. Гарантируется, что если на реке осталось единственное предприятие, оно не банкротится.

Выходные данные
Выходной файл должен содержать (k+1) целых чисел, по одному в строке. Первая строка должна содержать исходную сумму квадратов длин отрезков реки, а каждая из последующих k строк — сумму квадратов длин отрезков реки после очередного события.

Пример
входные данные
4 0
3 5 5 4
5
1 1
2 1
1 3
2 2
1 3
выходные данные
75
105
73
101
83
113

*/


#include <cstdlib>
#include <cstdio>

#ifdef ALGO_DEBUG
#include <string>  // For to_string for <T> dump
#endif

#ifdef ALGO_DEBUG
#include <cassert>
#else
#define assert(stmt)
#endif


namespace abel {
    // So, this is probably going to be an abomination of explicit and implicit treap, and users
    // will have to be extremely careful not to use the methods of the other type, all this to avoid inheritance...

    template <typename T> class Treap;  // Forward declaration for befriending


    template <typename T>
    class TreapNode {
        friend class Treap<T>;
    public:
        T key;
        T sqrSum;

        static TreapNode<T> *const STUB;

        static TreapNode<T> *create() {
            TreapNode<T> *self = new TreapNode<T>();
            assert(self);

            return self->ctor();
        }

        TreapNode<T> *ctor() {
            key = {};
            sqrSum = 0;
            size = 0;
            // I guess we want to make sure that STUB elements are also leaves by the heap criteria...
            priority = (unsigned long long)-1;
            children[0] = nullptr;
            children[1] = nullptr;

            return this;
        }

        TreapNode<T> *ctor(T new_key) {
            key = new_key;
            sqrSum = key * key;
            size = 1;
            priority = (rand() << 16) | rand();
            children[0] = STUB;
            children[1] = STUB;

            return this;
        }

        TreapNode<T> *ctor(T new_key, TreapNode<T> *new_left, TreapNode<T> *new_right) {
            key = new_key;
            sqrSum = key * key;
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
                printf(" (%s, %u, %s):\n", std::to_string(key).c_str(), size, std::to_string(sqrSum).c_str());

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
        // T squareSum;  // TODO

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

            sqrSum = key * key + left()->sqrSum + right()->sqrSum;
        }
    };


    template <typename T>
    class Treap {
    public:
        static Treap<T> *create() {
            Treap<T> *self = new Treap<T>();
            assert(self);

            return self->ctor();
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

        T getSqrSum() {
            return root->sqrSum;
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

        // This task was done in more of a hurry, so the interfaces are a bit less comfortable

        void handleBankrupcy(unsigned ind) {
            //printf("B>> %u\n", ind);

            struct TreapNode<T>::Pair result = {};

            result = root->splitInd(ind);
            root = result.left;
            TreapNode<T> *midSubtree = result.right;

            /*printf("Left Pre:\n");
            root->dump(1);
            printf("Mid Pre:\n");
            midSubtree->dump(1);*/

            result = midSubtree->splitInd(1);
            midSubtree = result.left;
            TreapNode<T> *rightSubtree = result.right;

            /*printf("Left:\n");
            root->dump(1);
            printf("Mid:\n");
            midSubtree->dump(1);
            printf("Right:\n");
            rightSubtree->dump(1);*/

            if (root->isStub()) {
                rightSubtree->set(0, rightSubtree->get(0) + midSubtree->get(0));
            } else if (rightSubtree->isStub()) {
                root->set(root->size - 1, root->get(root->size - 1) + midSubtree->get(0));
            } else {
                root->set(root->size - 1, root->get(root->size - 1) + midSubtree->get(0) / 2);
                rightSubtree->set(0, rightSubtree->get(0) + (midSubtree->get(0) + 1) / 2);
            }

            root = TreapNode<T>::merge({root, TreapNode<T>::merge({midSubtree->erase(), rightSubtree})});  // TODO: ?

            //root = TreapNode<T>::merge({root, rightSubtree});
        }

        void handleSplit(unsigned ind) {
            //printf("S>> %u\n", ind);

            struct TreapNode<T>::Pair result = {};

            result = root->splitInd(ind);
            root = result.left;
            TreapNode<T> *midSubtree = result.right;

            result = midSubtree->splitInd(1);
            midSubtree = result.left;
            TreapNode<T> *rightSubtree = result.right;

            midSubtree = midSubtree->insertInd(1, midSubtree->get(0) / 2);
            midSubtree = midSubtree->insertInd(2, (midSubtree->get(0) + 1) / 2);
            midSubtree = midSubtree->eraseInd(0);

            midSubtree = TreapNode<T>::merge({midSubtree, rightSubtree});
            root = TreapNode<T>::merge({root, midSubtree});
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

template <typename T>
abel::TreapNode<T> abel::TreapNode<T>::STUBVAL{};

template <typename T>
abel::TreapNode<T> *const abel::TreapNode<T>::STUB = &abel::TreapNode<T>::STUBVAL;


int main() {
    typedef long long Item;
    typedef abel::Treap<Item> Tree;
    typedef abel::TreapNode<Item> Node;

    srand(123);
    FILE *ifile = fopen("river.in", "r");
    #ifndef ALGO_DEBUG
    FILE *ofile = fopen("river.out", "w");
    #else
    FILE *ofile = stdout;
    #endif

    int res = 0;

    unsigned n = 0;
    res = fscanf(ifile, "%u %*d", &n);
    assert(res == 1);

    Tree tree;
    tree.ctor();

    for (unsigned i = 0; i < n; ++i) {
        #ifdef ALGO_DEBUG
        tree.dump();
        #endif // ALGO_DEBUG

        Item tmp = 0;
        res = fscanf(ifile, "%lld", &tmp);
        assert(res == 1);

        tree.insertInd(i + 1, tmp);
    }

    unsigned k = 0;
    res = fscanf(ifile, "%u", &k);
    assert(res == 1);

    for (unsigned i = 0; i < k; ++i) {
        #ifdef ALGO_DEBUG
        tree.dumpLinear();
        tree.dump();
        #endif // ALGO_DEBUG

        fprintf(ofile, "%lld\n", tree.getSqrSum());

        int cmd = 0;
        unsigned arg = 0;

        res = fscanf(ifile, "%d %u", &cmd, &arg);
        assert(res == 2);

        arg--;

        switch (cmd) {
        case 1:
            tree.handleBankrupcy(arg);
            break;
        case 2:
            tree.handleSplit(arg);
            break;
        default:
            assert(false);
            return 1;
        }
    }

    fprintf(ofile, "%lld\n", tree.getSqrSum());

    tree.dtor();
    fclose(ifile);
    fclose(ofile);

    return 0;
}


/*

Complexity: O(n log n), just like with every other task

*/


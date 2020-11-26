//#define ALGO_DEBUG

/*

C. K-ый максимум
ограничение по времени на тест0.5 секунд
ограничение по памяти на тест64 мегабайта
вводстандартный ввод
выводстандартный вывод
Напишите программу, реализующую структуру данных, позволяющую добавлять и удалять элементы, а также находить k-й максимум.

Входные данные
Первая строка входного файла содержит натуральное число n — количество команд (n\N{LESS-THAN OR EQUAL TO}100000). Последующие n строк содержат по одной команде каждая. Команда записывается в виде двух чисел ci и ki — тип и аргумент команды соответственно (|ki|\N{LESS-THAN OR EQUAL TO}109).

Поддерживаемые команды:

+1 (или просто 1): Добавить элемент с ключом ki.
   0: Найти и вывести ki-й максимум.
\N{MINUS SIGN}1: Удалить элемент с ключом ki.
Гарантируется, что в процессе работы в структуре не требуется хранить элементы с равными ключами или удалять несуществующие элементы. Также гарантируется, что при запросе ki-го максимума, он существует.

Выходные данные
Для каждой команды нулевого типа в выходной файл должна быть выведена строка, содержащая единственное число — ki-й максимум.

Пример
входные данные
11
+1 5
+1 3
+1 7
0 1
0 2
0 3
-1 5
+1 10
0 1
0 2
0 3
выходные данные
7
5
3
10
7
3

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
    template <typename T> class AVLTree;  // Forward declaration for befriending


    template <typename T>
    class AVLNode {
        // We need it this way, because AVLTree has a destructor with recursive deletion of nodes implemented in it, so it needs access to children
        friend class AVLTree<T>;
    public:
        T key;

        static AVLNode<T> *const STUB;

        static AVLNode<T> *create() {
            AVLNode<T> *self = new AVLNode<T>();
            assert(self);

            return self->ctor();
        }

        AVLNode<T> *ctor() {
            key = {};
            height = 0;
            size = 0;
            children[0] = nullptr;
            children[1] = nullptr;

            return this;
        }

        AVLNode<T> *ctor(T new_key) {
            key = new_key;
            height = 1;
            size = 1;
            children[0] = STUB;
            children[1] = STUB;

            return this;
        }

        AVLNode<T> *ctor(T new_key, AVLNode<T> *new_left, AVLNode<T> *new_right) {
            key = new_key;
            height = 1;
            size = 0;
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
            return height == 0;
        }

        inline int getHeight() {
            return height;
        }

        inline unsigned getSize() {
            return size;
        }

        AVLNode<T> *edgeChild(bool isRight) {
            assert(!isStub());

            if (!children[isRight]->isStub())
                return children[isRight]->edgeChild(isRight);

            return this;
        }

        // Convenience wrappers
        AVLNode<T> *min() {
            return edgeChild(false);
        }

        AVLNode<T> *max() {
            return edgeChild(true);
        }

        AVLNode<T> *erase() {
            if (isStub()) return STUB;

            AVLNode<T> *lNode = left();
            AVLNode<T> *rNode = right();  // Kind of caching, because we'll delete 'this' in several cases
            left() = STUB;
            right() = STUB;

            if (lNode->isStub() && rNode->isStub()) {
                delete this;
                return STUB;
            }

            if (!lNode->isStub() && rNode->isStub()) {
                delete this;
                return lNode;
            }

            if (lNode->isStub() && !rNode->isStub()) {
                delete this;
                return rNode;
            }

            AVLNode<T> *replacement = lNode->max();  // The rightmost child of the left subtree
            assert(!replacement->isStub());  // At least one node must exist there

            left() = left()->unlinkChild(replacement->key);

            replacement->right() = right();
            replacement->left() = left();

            delete this;

            replacement->fix();

            return replacement->balance();
        }

        AVLNode<T> *erase(T goal_key) {
            if (isStub()) return STUB;

            if (key == goal_key) return erase();

            bool isRight = goal_key > key;

            children[isRight] = children[isRight]->erase(goal_key);

            fix();

            return balance();
        }

        AVLNode<T> *insert(T new_key) {
            //printf("> (%d)\n", key);

            if (isStub()) return AVLNode<T>::create()->ctor(new_key);

            if (new_key == key) return this;

            bool isRight = new_key > key;

            children[isRight] = children[isRight]->insert(new_key);

            fix();

            return balance();
        }

        AVLNode<T> *find(T goal_key) {
            if (isStub()) return STUB;

            if (key == goal_key) return this;

            return children[goal_key > key]->find(goal_key);
        }

        AVLNode<T> *findClosest(T goal_key, bool isRight) {
            /*printf("> %p ", this);
            if (this) printf("(key=%d)", key);
            printf("\n");*/

            if (isStub()) return STUB;

            if (key == goal_key) {  // Maybe an unnecessary optimization, but why not
                return children[isRight]->edgeChild(!isRight);
            }

            AVLNode<T> *result = children[goal_key > key]->findClosest(goal_key, isRight);

            if (result->isStub() && (isRight == (goal_key < key))) return this;  // This essentially says that if the goal subtree is empty, the current node is the goal

            return result;
        }

        AVLNode<T> *minAbove(T goal_key) {
            return findClosest(goal_key, true);
        }

        AVLNode<T> *maxBelow(T goal_key) {
            return findClosest(goal_key, false);
        }

        AVLNode<T> *kthElem(unsigned k) {
            if (isStub()) {
                return STUB;
            }

            unsigned rSize = right()->size;

            if (k < rSize) {
                return right()->kthElem(k);
            } else if (k == rSize) {
                return this;
            } else {
                return left()->kthElem(k - rSize - 1);
            }
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
                printf(" (%s, %d, %u):\n", std::to_string(key).c_str(), height, size);

                left ()->dump(depth + 1);
                right()->dump(depth + 1);
            }
        }
        #endif // ALGO_DEBUG

    #ifndef ALGO_DEBUG
    protected:
    #endif
        int height;
        unsigned size;
        AVLNode<T> *children[2];  // {left, right}

        static AVLNode<T> STUBVAL;

        // The following two methods are provided solely for convenience purposes as aliases
        // for children[0] and children[1]. The same effect could have been achieved with
        // #define left children[0] and #define right children[1], but I decided
        // this approach to be a better code practice
        inline AVLNode<T> *&left() {
            assert(!isStub());

            return children[0];
        }

        inline AVLNode<T> *&right() {
            assert(!isStub());

            return children[1];
        }

        void fix() {
            if (isStub()) return;

            int  leftHeight =  left()->getHeight();
            int rightHeight = right()->getHeight();

            height = (leftHeight > rightHeight ? leftHeight : rightHeight) + 1;

            size = 1 + left()->size + right()->size;
        }

        // This way we get to generalize rotateLeft and rotateRight
        AVLNode<T> *rotateSmall(bool isRight) {
            assert(!isStub());

            AVLNode<T> *otherNode = children[!isRight];
            assert(!otherNode->isStub());

            children[!isRight] = otherNode->children[isRight];

            otherNode->children[isRight] = this;

            fix();
            otherNode->fix();

            return otherNode;
        }

        // Same deal
        AVLNode<T> *rotateGrand(bool isRight) {
            assert(!isStub());

            children[!isRight] = children[!isRight]->rotateSmall(!isRight);

            return rotateSmall(isRight);
        }

        int getBalanceFactor() {
            if (isStub()) return 0;

            int  leftHeight =  left()->getHeight();
            int rightHeight = right()->getHeight();

            assert(rightHeight - 2 <= leftHeight && rightHeight + 2 >= leftHeight);

            return rightHeight - leftHeight;
        }

        AVLNode<T> *balance() {
            assert(!isStub());

            bool isRight = false;
            int childBal = 0;

            switch (getBalanceFactor()) {
            case 0:
            case 1:
            case -1:
                return this;
            case 2:
                isRight = true;
                // The following comment suppresses a warning about implicit fallthrough
                // FALLTHROUGH
            case -2:
                childBal = children[isRight]->getBalanceFactor();
                childBal *= isRight ? 1 : -1;

                //printf(">> %d %c %d\n", key, isRight ? 'R' : 'L', childBal);

                if (childBal == 1 || childBal == 0) {
                    return rotateSmall(!isRight);
                } else {
                    return rotateGrand(!isRight);
                }
            default:
                assert(false /* Shouldn't be reachable */);  // Clear
                return STUB;
            }
        }

        AVLNode<T> *unlinkChild(T goal_key) {
            if (isStub()) return STUB;

            if (key == goal_key) {
                return !left()->isStub() ? left() : right();
            }

            bool isRight = goal_key > key;

            children[isRight] = children[isRight]->unlinkChild(goal_key);

            fix();

            return balance();
        }
    };


    template <typename T>
    class AVLTree {
    public:
        static AVLTree<T> *create() {
            AVLTree<T> *self = new AVLTree<T>();
            assert(self);

            return self->ctor();
        }

        AVLTree<T> *ctor() {
            root = AVLNode<T>::STUB;

            return this;
        }

        void dtor() {
            if (root && !root->isStub()) {
                root->dtor();
                free(root);
            }
        }

        AVLNode<T> *getRootSubtree() {
            // Whilst root as a field should probably be private, I can see people willing to make requests on subtrees,
            // so they probably have the right to obtain the root subtree as well
            return root;
        }

        void insert(T key) {
            root = root->insert(key);
        }

        void erase(T key) {
            root = root->erase(key);
        }

        bool isEmpty() {
            return root->isStub();
        }

        AVLNode<T> *find(T key) {
            return root->find(key);
        }

        bool contains(T key) {
            return !find(key)->isStub();
        }

        T max() {
            assert(!root->isStub());

            return root->max()->key;
        }

        T min() {
            assert(!root->isStub());

            return root->min()->key;
        }

        // Since we don't know if T has a poisonous value, we'll have to return key in
        // the following two functions in case of absence of the requested element
        T maxBelow(T key) {
            if (root->isStub()) {
                return key;
            }

            AVLNode<T> *result = root->maxBelow(key);

            return !result->isStub() ? result->key : key;
        }

        T minAbove(T key) {
            if (root->isStub()) {
                return key;
            }

            AVLNode<T> *result = root->minAbove(key);

            return !result->isStub() ? result->key : key;
        }

        T kthElem(unsigned k) {
            AVLNode<T> *result = root->kthElem(k);

            assert(!result->isStub());

            return result->key;
        }

        #ifdef ALGO_DEBUG
        void dump() {
            printf("AVLTree [%p] {\n", this);
            root->dump(1);
            printf("}\n");
        }
        #endif // ALGO_DEBUG

    #ifndef ALGO_DEBUG
    protected:
    #endif
        AVLNode<T> *root;
    };
}


template <typename T>
abel::AVLNode<T> abel::AVLNode<T>::STUBVAL{};

template <typename T>
abel::AVLNode<T> *const abel::AVLNode<T>::STUB = &abel::AVLNode<T>::STUBVAL;


int main() {
    typedef long long Item;
    typedef abel::AVLTree<Item> Tree;
    typedef abel::AVLNode<Item> Node;

    int res = 0;
    unsigned n = 0;

    res = scanf("%u", &n);
    assert(res == 1);

    Tree tree;
    tree.ctor();

    for (unsigned i = 0; i < n; ++i) {
        #ifdef ALGO_DEBUG
        tree->dump();
        #endif // ALGO_DEBUG

        int cmd = 0;
        Item arg = 0;

        res = scanf("%d %lld", &cmd, &arg);
        assert(res == 2);

        switch (cmd) {
        case 1:
            tree.insert(arg);
            break;
        case 0:
            printf("%lld\n", tree.kthElem((unsigned)arg - 1));  // I know an explicit cast isn't required, but it just feels reasonable here
            break;
        case -1:
            tree.erase(arg);
            break;
        default:
            assert(false);
            return 1;
        }
    }

    tree.dtor();

    return 0;
}


/*

Complexity: Come on. It's a tree task, they all work in O(n log n)

*/

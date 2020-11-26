/*

Реализуйте AVL-дерево. Решения с использованием других структур засчитываться не будут.

Входные данные
Входной файл содержит описание операций с деревом. Операций не больше 105.

В каждой строке находится одна из следующих операций:

insert x — добавить в дерево ключ x.
delete x — удалить из дерева ключ x. Если ключа x в дереве нет, то ничего делать не надо.
exists x — если ключ x есть в дереве, вывести «true», иначе «false»
next x — минимальный элемент в дереве, больший x, или «none», если такого нет.
prev x — максимальный элемент в дереве, меньший x, или «none», если такого нет.
Все числа во входном файле целые и по модулю не превышают 109.

Выходные данные
Выведите последовательно результат выполнения всех операций exists, next, prev.

Следуйте формату выходного файла из примера.

Пример
входные данные
insert 2
insert 5
insert 3
exists 2
exists 4
next 4
prev 4
delete 5
next 4
prev 4
выходные данные
true
false
5
3
none
3

*/

//#define ALGO_DEBUG

#include <cstdio>
#include <cstdlib>

#ifdef ALGO_DEBUG
#include <string>  // For to_string for <T> dump
#endif

#ifdef ALGO_DEBUG
#include <cassert>
#else
#define assert(stmt)
#endif



/*
 It turned out that nullptr 'this' doesn't work on CF, so I'll have to either implement an abstract base AVL node class
 from which will inherit a class for regular nodes and a stub class - but that would probably be extremely slow - or
 maintain a static AVLNode<T> *STUB - which is probably what I'll stick to doing.
*/

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
            children[0] = nullptr;
            children[1] = nullptr;

            return this;
        }

        AVLNode<T> *ctor(T new_key) {
            key = new_key;
            height = 1;
            children[0] = STUB;
            children[1] = STUB;

            return this;
        }

        AVLNode<T> *ctor(T new_key, AVLNode<T> *new_left, AVLNode<T> *new_right) {
            key = new_key;
            height = 1;
            children[0] = new_left;
            children[1] = new_right;

            updateHeight();

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

            replacement->updateHeight();

            return replacement->balance();
        }

        AVLNode<T> *erase(T goal_key) {
            if (isStub()) return STUB;

            if (key == goal_key) return erase();

            bool isRight = goal_key > key;

            children[isRight] = children[isRight]->erase(goal_key);

            updateHeight();

            return balance();
        }

        AVLNode<T> *insert(T new_key) {
            //printf("> (%d)\n", key);

            if (isStub()) return AVLNode<T>::create()->ctor(new_key);

            if (new_key == key) return this;

            bool isRight = new_key > key;

            children[isRight] = children[isRight]->insert(new_key);

            updateHeight();

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

        #ifdef ALGO_DEBUG
        void dump(int depth = 0) {
            for (int i = 0; i < depth; ++i) {
                printf("  ");
            }
            printf("[%p]", this);
            if (isStub()) {
                printf(" (STUB)\n");
            } else {
                printf(" (%s):\n", std::to_string(key).c_str());

                left ()->dump(depth + 1);
                right()->dump(depth + 1);
            }
        }
        #endif // ALGO_DEBUG

    #ifndef ALGO_DEBUG
    protected:
    #endif
        int height;
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

        void updateHeight() {
            if (isStub()) return;

            int  leftHeight =  left()->getHeight();
            int rightHeight = right()->getHeight();

            height = (leftHeight > rightHeight ? leftHeight : rightHeight) + 1;
        }

        // This way we get to generalize rotateLeft and rotateRight
        AVLNode<T> *rotateSmall(bool isRight) {
            assert(!isStub());

            AVLNode<T> *otherNode = children[!isRight];
            assert(!otherNode->isStub());

            children[!isRight] = otherNode->children[isRight];

            otherNode->children[isRight] = this;

            updateHeight();
            otherNode->updateHeight();

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

            updateHeight();

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
            return find(key) != AVLNode<T>::STUB;
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


static const int CMD_LEN = 8;
static const char CMD_FMT[] = "%7s";

int main() {
    abel::AVLTree<int> tree;

    tree.ctor();

    char cmd[CMD_LEN] = "";

    while (!feof(stdin)) {
        #ifdef ALGO_DEBUG
        tree.dump();
        #endif

        int res = scanf(CMD_FMT, cmd);
        if (res != 1) break;

        #ifdef ALGO_DEBUG
        if (strcmp(cmd, "dump") == 0) {
            tree.dump();
            continue;
        } else if (strcmp(cmd, "exit") == 0) {
            break;
        }
        #endif

        int arg = 0;
        res = scanf("%d", &arg);
        if (res != 1) break;

        int tmp = 0;  // I can't declare it inside a 'case'

        switch (cmd[0]) {  // This is way more efficient than consequent strcmps
        case 'i':  // insert
            tree.insert(arg);

            break;
        case 'd':  // delete
            tree.erase(arg);

            break;
        case 'e':  // exists
            printf("%s\n", tree.contains(arg) ? "true" : "false");

            break;
        case 'n':  // next
            tmp = tree.minAbove(arg);

            if (tmp == arg) {
                printf("none\n");
            } else {
                printf("%d\n", tmp);
            }

            break;
        case 'p':  // prev
            tmp = tree.maxBelow(arg);

            if (tmp == arg) {
                printf("none\n");
            } else {
                printf("%d\n", tmp);
            }

            break;
        #ifdef ALGO_DEBUG
        case 'b':  // balance
            {
                abel::AVLNode<int> *root = tree.find(arg);

                printf("Balance before: %d\n", root->getBalanceFactor());
                root->balance();
                printf("Balance after : %d\n", root->getBalanceFactor());
            }

            break;
        #endif
        default:
            tree.dtor();
            assert(false /* Shouldn't be reachable */);
            return 1;
        }
    }

    tree.dtor();

    return 0;
}


/*

Complexity: Coding in C++ was super easy)). To be serious, O(n log n), because it's just n requests to an AVL tree, whose size can't exceed n.

*/


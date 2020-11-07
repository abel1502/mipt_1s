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

#ifdef ALGO_DEBUG
#include <TXLib.h>
#pragma GCC diagnostic ignored "-Weffc++"  // While I appreciate the advice, I don't think I'll be providing a tree copy constructor, sorry)
#endif

#include <cstdio>
#include <cstdlib>
#include <cassert>

#ifndef ALGO_DEBUG
#define NDEBUG
#endif


namespace abel {
    template <typename T> class AVLTree;  // Forward declaration for befriending

    template <typename T>
    class AVLNode {
        // We need it this way, because AVLTree has a destructor with recursive deletion of nodes implemented in it, so it needs access to children
        friend class AVLTree<T>;
    public:
        T key;

        AVLNode(T new_key) : key(new_key), height(1), children{nullptr, nullptr} {};

        AVLNode(T new_key, AVLNode<T> *left, AVLNode<T> *right) : key(new_key), height(1), children{left, right} {};

        int getHeight() {
            // This check gives a warning for some reason, however I have tested that
            //'this' may, in fact, be nullptr if I call a method on a nullptr-child
            if (!this) return 0;

            return height;
        }

        AVLNode<T> *edgeChild(bool isRight) {
            assert(this);

            if (children[isRight])
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
            if (!this) return nullptr;

            AVLNode<T> *lNode = left();
            AVLNode<T> *rNode = right();  // Kind of caching, because we'll delete 'this' in several cases

            if (!lNode && !rNode) {
                delete this;
                return nullptr;
            }

            if (lNode && !rNode) {
                delete this;
                return lNode;
            }

            if (!lNode && rNode) {
                delete this;
                return rNode;
            }

            AVLNode<T> *replacement = lNode->edgeChild(true);  // The rightmost child of the left subtree
            assert(replacement);  // At least one node must exist there

            T tmpKey = replacement->key;
            replacement->key = key;
            key = tmpKey;

            replacement->erase();  // We have to do this because replacement isn't guaranteed to be a leaf:
            /*
                3 <----- If we're erasing this, 2 is the replacement
              2   4
            1
            */

            updateHeight();

            return balance();
        }

        AVLNode<T> *erase(T goal_key) {
            if (!this) return nullptr;

            if (key == goal_key) return erase();

            bool isRight = goal_key > key;

            children[isRight] = children[isRight]->erase(goal_key);

            updateHeight();

            return balance();
        }

        AVLNode<T> *insert(T new_key) {
            if (!this) return new AVLNode<T>(new_key);

            bool isRight = new_key > key;

            children[isRight] = children[isRight]->insert(new_key);

            updateHeight();

            return balance();
        }

        AVLNode<T> *find(T goal_key) {
            if (!this) return nullptr;

            if (key == goal_key) return this;

            return children[goal_key > key]->find(goal_key);
        }

        AVLNode<T> *findClosest(T goal_key, bool isRight) {
            /*printf("> %p ", this);
            if (this) printf("(key=%d)", key);
            printf("\n");*/

            if (!this) return nullptr;

            if (key == goal_key) {  // Maybe an unnecessary optimization, but why not
                return children[isRight] ? children[isRight]->edgeChild(!isRight) : nullptr;
            }

            AVLNode<T> *result = children[goal_key > key]->findClosest(goal_key, isRight);

            if (!result && (isRight == (goal_key < key))) return this;  // This essentially says that if the goal subtree is empty, the current node is the goal

            return result;
        }

        AVLNode<T> *minAbove(T goal_key) {
            return findClosest(goal_key, true);
        }

        AVLNode<T> *maxBelow(T goal_key) {
            return findClosest(goal_key, false);
        }
    #ifndef ALGO_DEBUG
    protected:
    #endif
        int height;
        AVLNode<T> *children[2];  // {left, right}

        // The following two methods are provided solely for convenience purposes as aliases
        // for children[0] and children[1]. The same effect could have been achieved with
        // #define left children[0] and #define right children[1], but I decided
        // this approach to be a better code practice
        inline AVLNode<T> *&left() {
            assert(this);

            return children[0];
        }

        inline AVLNode<T> *&right() {
            assert(this);

            return children[1];
        }

        void updateHeight() {
            if (!this) return;

            int  leftHeight =  left()->getHeight();
            int rightHeight = right()->getHeight();

            height = (leftHeight > rightHeight ? leftHeight : rightHeight) + 1;
        }

        // This way we get to generalize rotateLeft and rotateRight
        AVLNode<T> *rotateSmall(bool isRight) {
            assert(this);

            AVLNode<T> *otherNode = children[!isRight];
            assert(otherNode);

            children[!isRight] = otherNode->children[isRight];

            otherNode->children[isRight] = this;

            return otherNode;
        }

        // Same deal
        AVLNode<T> *rotateGrand(bool isRight) {
            assert(this);

            children[!isRight] = children[!isRight]->rotateSmall(!isRight);

            return rotateSmall(isRight);
        }

        int getBalanceFactor() {
            if (!this) return 0;

            int  leftHeight =  left()->getHeight();
            int rightHeight = right()->getHeight();

            assert(rightHeight - 2 <= leftHeight && rightHeight + 2 >= leftHeight);

            return rightHeight - leftHeight;
        }

        AVLNode<T> *balance() {
            assert(this);

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

                if (childBal <= 1) {
                    return rotateSmall(!isRight);
                } else {
                    return rotateGrand(!isRight);
                }
            default:
                assert(false /* Shouldn't be reachable */);
                return nullptr;
            }
        }
    };

    template <typename T>
    class AVLTree {
    public:
        AVLTree() : root(nullptr) {}

        ~AVLTree() {
            recursiveDelete(root);
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

        bool empty() {
            return root != nullptr;
        }

        AVLNode<T> *find(T key) {
            return root->find(key);
        }

        bool contains(T key) {
            return find(key) != nullptr;
        }

        T max() {
            assert(root);

            return root->max().key;
        }

        T min() {
            assert(root);

            return root->min().key;
        }

        // Since we don't know if T has a poisonous value, we'll have to return key in
        // the following two functions in case of absence of the requested element
        T maxBelow(T key) {
            assert(root);

            AVLNode<T> *result = root->maxBelow(key);

            return result ? result->key : key;
        }

        T minAbove(T key) {
            assert(root);

            AVLNode<T> *result = root->minAbove(key);

            return result ? result->key : key;
        }
    #ifndef ALGO_DEBUG
    protected:
    #endif
        AVLNode<T> *root;

        void recursiveDelete(AVLNode<T> *node) {
            assert(this);

            if (!node) return;

            recursiveDelete(node->left());
            recursiveDelete(node->right());

            delete node;
        }
    };  // TODO: Dump?
}


static const int CMD_LEN = 8;
static const char CMD_FMT[] = "%7s";


int main() {
    abel::AVLTree<int> tree;

    char cmd[CMD_LEN] = "";

    while (!feof(stdin)) {
        int res = scanf(CMD_FMT, cmd);
        assert(res == 1);

        int arg = 0;
        res = scanf("%d", &arg);
        assert(res == 1);

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
        default:
            assert(false /* Shouldn't be reachable */);
            return 1;
        }
    }

    return 0;
}


/*

Complexity: Coding in C++ was super easy)). To be serious, O(n log n), because it's just n requests to an AVL tree, whose size can't exceed n.

*/


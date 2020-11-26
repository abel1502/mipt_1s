//#define ALGO_DEBUG

/*

Реализуйте splay-дерево, которое поддерживает множество S целых чисел, в котором разрешается производить следующие операции:

add(i) — добавить в множество S число i (если он там уже есть, то множество не меняется);
sum(l,r) — вывести сумму всех элементов x из S, которые удовлетворяют неравенству l\N{LESS-THAN OR EQUAL TO}x\N{LESS-THAN OR EQUAL TO}r.
Решения, не использующие splay-деревья, будут игнорироваться.

Решения, не использующие операции split и merge, будут оцениваться в 2 балла. Решения, использующие операции split и merge, оцениваются в 3 балла.

Входные данные
Исходно множество S пусто. Первая строка входного файла содержит n — количество операций (1\N{LESS-THAN OR EQUAL TO}n\N{LESS-THAN OR EQUAL TO}300000). Следующие n строк содержат операции. Каждая операция имеет вид либо «+ i», либо «? l r». Операция «? l r» задает запрос sum(l,r).

Если операция «+ i» идет во входном файле в начале или после другой операции «+», то она задает операцию add(i). Если же она идет после запроса «?», и результат этого запроса был y, то выполняется операция add((i+y)mod109).

Во всех запросах и операциях добавления параметры лежат в интервале от 0 до 109.

Выходные данные
Для каждого запроса выведите одно число — ответ на запрос.

Пример
входные данные
6
+ 1
+ 3
+ 3
? 2 4
+ 1
? 2 4
выходные данные
3
7

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
    template <typename T> class SplayTree;  // Forward declaration for befriending


    template <typename T>
    class SplayNode {
        // Necessary for split and merge within SplayTree instead of SplayNode
        friend class SplayTree<T>;
    public:
        T key;

        static SplayNode<T> *const STUB;

        static SplayNode<T> *create() {
            SplayNode<T> *self = new SplayNode<T>();
            assert(self);

            return self->ctor();
        }

        SplayNode<T> *ctor() {
            key = {};
            isRightChild = false;
            parent = nullptr;
            children[0] = nullptr;
            children[1] = nullptr;
            sum = key;

            return this;
        }

        SplayNode<T> *ctor(T new_key) {
            key = new_key;
            isRightChild = false;  // Will be updated in fix()
            parent = STUB;
            children[0] = STUB;
            children[1] = STUB;
            sum = key;

            return this;
        }

        SplayNode<T> *ctor(T new_key, SplayNode<T> *new_left, SplayNode<T> *new_right) {
            key = new_key;
            isRightChild = false;  // Will be updated in fix()
            parent = STUB;
            children[0] = new_left;
            children[1] = new_right;
            sum = key + left()->sum + right()->sum;

            // fix();  // I'm not sure if I should implicitly call it here or let the user call it when they're ready...
                       // TODO: Maybe should make this constructor private

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
            return children[0] == nullptr;
        }

        SplayNode<T> *edgeChild(bool isRight) {
            assert(!isStub());

            if (!children[isRight]->isStub())
                return children[isRight]->edgeChild(isRight);

            return this;
        }

        // Convenience wrappers
        SplayNode<T> *min() {
            return edgeChild(false);
        }

        SplayNode<T> *max() {
            return edgeChild(true);
        }

        SplayNode<T> *erase() {
            if (isStub()) return STUB;

            SplayNode<T> *lNode = left();
            SplayNode<T> *rNode = right();  // Kind of caching, because we'll delete 'this' in several cases
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

            SplayNode<T> *replacement = lNode->max();  // The rightmost child of the left subtree
            assert(!replacement->isStub());  // At least one node must exist there
            assert(replacement->left()->isStub() || replacement->right()->isStub());  // And it can't have two children

            replacement->parent->children[replacement->isRightChild] = \
                replacement->left()->isStub() ? replacement->right() : replacement->left();

            replacement->right() = rNode;
            replacement->left() = lNode;

            replacement->parent->fix();  // Old parent
            replacement->fix();

            SplayNode<T> *newRoot = parent;

            delete this;

            newRoot->splay();

            return replacement;
        }

        SplayNode<T> *erase(T goal_key) {
            if (isStub()) return STUB;

            if (key == goal_key) return erase();

            bool isRight = goal_key > key;

            children[isRight] = children[isRight]->erase(goal_key);

            fix();

            return this;  // TODO: Maybe remove unnecessary returns
        }

        SplayNode<T> *insert(T new_key) {
            //printf("> (%d)\n", key);

            if (isStub()) return SplayNode<T>::create()->ctor(new_key);

            if (new_key == key) return this;

            bool isRight = new_key > key;

            bool isLeaf = children[isRight]->isStub();

            SplayNode<T> *newChild = children[isRight]->insert(new_key);

            if (isLeaf) {
                children[isRight] = newChild;
            }

            fix();

            return newChild;
        }

        SplayNode<T> *find(T goal_key) {
            if (isStub()) return STUB;

            if (key == goal_key) return this;

            return children[goal_key > key]->find(goal_key);
        }

        SplayNode<T> *findClosest(T goal_key, bool isRight) {
            /*printf("> %p ", this);
            if (this) printf("(key=%d)", key);
            printf("\n");*/

            if (isStub()) return STUB;

            if (key == goal_key) {  // Maybe an unnecessary optimization, but why not
                return children[isRight]->edgeChild(!isRight);
            }

            SplayNode<T> *result = children[goal_key > key]->findClosest(goal_key, isRight);

            if (result->isStub() && (isRight == (goal_key < key))) return this;  // This essentially says that if the goal subtree is empty, the current node is the goal

            return result;
        }

        SplayNode<T> *minAbove(T goal_key) {
            return findClosest(goal_key, true);
        }

        SplayNode<T> *maxBelow(T goal_key) {
            return findClosest(goal_key, false);
        }

        T getSum() {
            return sum;
        }

        #ifdef ALGO_DEBUG
        void dump(int depth = 0) {
            for (int i = 0; i < depth; ++i) {
                printf("--");
            }
            printf("[%p]", this);
            if (isStub()) {
                printf(" (STUB)\n");
            } else {
                //printf(" (%s):\n", std::to_string(key).c_str());
                printf(" (%s, %s):\n", std::to_string(key).c_str(), std::to_string(sum).c_str());  // TODO: Remove

                assert(left ()->parent == this || left ()->isStub());
                assert(right()->parent == this || right()->isStub());

                left ()->dump(depth + 1);
                right()->dump(depth + 1);
            }
        }
        #endif // ALGO_DEBUG

    #ifndef ALGO_DEBUG
    protected:
    #endif
        bool isRightChild;
        SplayNode<T> *parent;
        SplayNode<T> *children[2];  // {left, right}
        T sum;

        static SplayNode<T> STUBVAL;

        // The following two methods are provided solely for convenience purposes as aliases
        // for children[0] and children[1]. The same effect could have been achieved with
        // #define left children[0] and #define right children[1], but I decided
        // this approach to be a better code practice
        inline SplayNode<T> *&left() {
            assert(!isStub());

            return children[0];
        }

        inline SplayNode<T> *&right() {
            assert(!isStub());

            return children[1];
        }

        void fix() {
            if (isStub()) return;

            sum = key;

            for (int i = 0; i < 2; ++i) {
                if (children[i]->isStub())  continue;

                children[i]->parent = this;
                children[i]->isRightChild = i == 1;
                sum += children[i]->sum;
            }

            if (!parent->isStub()) {
                isRightChild = parent->right() == this;
            }
        }

        // This way we get to generalize rotateLeft and rotateRight
        void rotateSmall(bool isRight) {
            assert(!isStub());

            SplayNode<T> *otherNode = children[!isRight];
            assert(!otherNode->isStub());

            children[!isRight] = otherNode->children[isRight];

            otherNode->children[isRight] = this;

            otherNode->parent = parent;
            if (!parent->isStub()) {
                parent->children[isRightChild] = otherNode;
            }
            parent = otherNode;

            fix();
            otherNode->fix();
            otherNode->parent->fix();
        }

        /*  It seems that we don't need rotateGrand in a splay tree
        // Same deal
        void rotateGrand(bool isRight) {
            assert(!isStub());

            children[!isRight]->rotateSmall(!isRight);
            rotateSmall(isRight);
        }
        */

        void splay() {
            assert(!isStub());

            while (!parent->isStub()) {  // TODO: Maybe encapsulate into isRoot
                SplayNode<T> *grandParent = parent->parent;

                if (grandParent->isStub()) {
                    parent->rotateSmall(!isRightChild);
                } else if (isRightChild == parent->isRightChild) {
                    // Zig-Zig (Haven't encapsulated for performance purposes and since it's only used once)
                    grandParent->rotateSmall(!isRightChild);
                    parent     ->rotateSmall(!isRightChild);
                } else {
                    // Zig-Zag (Same)
                    parent     ->rotateSmall(!isRightChild);
                    grandParent->rotateSmall(!isRightChild);  // This is actually the opposite direction, as it will have changed
                }
            }

            /*printf(">>> \n");
            dump();*/
        }
    };


    template <typename T>
    class SplayTree {  // TODO: Finish splayifying tree, split, merge
    public:
        static SplayTree<T> *create() {
            SplayTree<T> *self = new SplayTree<T>();
            assert(self);

            return self->ctor();
        }

        SplayTree<T> *ctor() {
            root = SplayNode<T>::STUB;

            return this;
        }

        void dtor() {
            if (root && !isEmpty()) {
                root->dtor();
                free(root);
            }
        }

        SplayNode<T> *getRootSubtree() {
            // Whilst root as a field should probably be private, I can see people willing to make requests on subtrees,
            // so they probably have the right to obtain the root subtree as well
            return root;
        }

        void insert(T key) {
            // TODO: Maybe reimplement through split & merge
            bool wasEmpty = isEmpty();

            SplayNode<T> *newNode = root->insert(key);

            if (wasEmpty) {
                root = newNode;
            } else {
                splay(newNode);
            }
        }

        void erase(T key) {
            root = root->erase(key);
        }

        bool isEmpty() {
            return root->isStub();
        }

        T getSum() {
            return root->getSum();
        }

        SplayNode<T> *find(T key) {
            SplayNode<T> *result = root->find(key);

            if (!result->isStub()) {
                splay(result);
            }

            return result;
        }

        bool contains(T key) {
            return find(key) != SplayNode<T>::STUB;
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
            if (isEmpty()) {
                return key;
            }

            SplayNode<T> *result = root->maxBelow(key);

            return !result->isStub() ? result->key : key;
        }

        T minAbove(T key) {
            if (isEmpty()) {
                return key;
            }

            SplayNode<T> *result = root->minAbove(key);

            return !result->isStub() ? result->key : key;
        }

        // For this task only, since it implies the possibility of subtraction of 1 from T
        // TODO: Maybe rework so that this isn't required?
        T getSumBetween(T leftEdge, T rightEdge) {
            SplayTree<T> *rightTree = split(rightEdge);
            SplayTree<T> *midTree = split(leftEdge - 1);

            T result = midTree->getSum();

            merge(midTree);
            merge(rightTree);

            rightTree->dtor();
            midTree->dtor();
            delete rightTree;
            delete midTree;

            return result;
        }

        #ifdef ALGO_DEBUG
        void dump() {
            printf("SplayTree [%p] {\n", this);
            root->dump(1);
            printf("}\n");
        }
        #endif // ALGO_DEBUG

    #ifndef ALGO_DEBUG
    protected:
    #endif
        SplayNode<T> *root;

        void splay(SplayNode<T> *targetNode) {
            assert(targetNode);

            targetNode->splay();
            root = targetNode;
        }

        // Split & merge aren't public because merge has to always be called after split, and on the same tree

        // Returns the right one of the new trees
        SplayTree<T> *split(T key) {
            if (find(key)->isStub()) {  // Automatically splays it, if it existed
                #ifdef ALGO_DEBUG
                printf("[Split by %lld]...\n", key);
                #endif // ALGO_DEBUG

                T new_key = maxBelow(key);

                if (new_key == key) {
                    #ifdef ALGO_DEBUG
                    printf("[Split trivial]\n");
                    #endif // ALGO_DEBUG

                    SplayTree *otherTree = SplayTree::create();

                    otherTree->root = root;
                    root = SplayNode<T>::STUB;

                    return otherTree;
                }

                return split(new_key);
            }

            SplayTree *otherTree = SplayTree::create();

            if (root->right()) {
                root->right()->parent = SplayNode<T>::STUB;  // This won't get auto-reset in fix(), so I'll have to do it manually
            }

            otherTree->root = root->right();
            root->right() = SplayNode<T>::STUB;

            otherTree->root->fix();
            root->fix();

            #ifdef ALGO_DEBUG
            printf("======== [Split by %-3lld] ========\n", key);
            dump();
            otherTree->dump();
            printf("=================================\n");
            #endif // ALGO_DEBUG

            return otherTree;
        }

        // Adds otherTree to the right
        void merge(SplayTree<T> *otherTree) {
            if (isEmpty()) {
                root = otherTree->root;
            } else if (!otherTree->isEmpty()) {
                splay(root->max());
                assert(root->right()->isStub());
                root->right() = otherTree->root;
            }

            root->fix();

            otherTree->root = SplayNode<T>::STUB;
        }
    };
}


template <typename T>
abel::SplayNode<T> abel::SplayNode<T>::STUBVAL{};

template <typename T>
abel::SplayNode<T> *const abel::SplayNode<T>::STUB = &abel::SplayNode<T>::STUBVAL;


int main() {
    typedef long long Item;
    typedef abel::SplayTree<Item> Tree;
    typedef abel::SplayNode<Item> Node;

    int res = 0;
    unsigned n = 0;

    res = scanf("%u", &n);
    assert(res == 1);

    Tree tree;

    tree.ctor();

    Item lastAnswer = 0;
    const Item MOD = 1000000000;

    for (unsigned i = 0; i < n; ++i) {
        #ifdef ALGO_DEBUG
        tree->dump();
        #endif // ALGO_DEBUG

        char cmd = '\0';

        getchar();
        res = scanf("%c", &cmd);
        assert(res == 1);

        switch (cmd) {
        case '+': {
            Item arg = 0;
            res = scanf("%lld", &arg);
            assert(res == 1);

            arg = (arg + lastAnswer) % MOD;
            lastAnswer = 0;

            tree.insert(arg);
        } break;
        case '?': {
            Item left = 0, right = 0;
            res = scanf("%lld %lld", &left, &right);
            assert(res == 2);

            printf("%lld\n", lastAnswer = tree.getSumBetween(left, right));
        } break;
        default:
            assert(false);
            return 1;
        }
    }

    tree.dtor();

    return 0;
}


/*

Commplexity: O(n log n), same as in A

*/

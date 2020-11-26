//#define ALGO_DEBUG

/*

E. Множества
ограничение по времени на тест0.75 секунд
ограничение по памяти на тест256 мегабайт
вводстандартный ввод
выводстандартный вывод
На вступительном контесте в пилотную группу по программированию Вашему другу предложили реализовать структуру данных для хранения множеств чисел. Так как он специализируется на истории литературы, данную структуру придётся реализовать Вам.

Структура должна хранить m+1 множеств чисел от 0 до n, пронумерованных от 0 до m включительно, при этом одно число может принадлежать сразу нескольким множествам. Изначально все множества пустые.

Вы должны реализовать следующие операции на этой структуре:

ADD e s
Добавить в множество №s (0\N{LESS-THAN OR EQUAL TO}s\N{LESS-THAN OR EQUAL TO}m) число e (0\N{LESS-THAN OR EQUAL TO}e\N{LESS-THAN OR EQUAL TO}n).

DELETE e s
Удалить из множества №s (0\N{LESS-THAN OR EQUAL TO}s\N{LESS-THAN OR EQUAL TO}m) число e (0\N{LESS-THAN OR EQUAL TO}e\N{LESS-THAN OR EQUAL TO}n). Гарантируется, что до этого число e было помещено в множество

CLEAR s
Очистить множество №s (0\N{LESS-THAN OR EQUAL TO}s\N{LESS-THAN OR EQUAL TO}m).

LISTSET s
Показать содержимое множества №s (0\N{LESS-THAN OR EQUAL TO}s\N{LESS-THAN OR EQUAL TO}m) в возрастающем порядке, либо \N{MINUS SIGN}1, если множество пусто.

LISTSETSOF e
Показать множества, в которых лежит число e (0\N{LESS-THAN OR EQUAL TO}e\N{LESS-THAN OR EQUAL TO}n), либо \N{MINUS SIGN}1, если этого числа нет ни в одном множестве.

Входные данные
Сначала вводятся числа N (1\N{LESS-THAN OR EQUAL TO}N\N{LESS-THAN OR EQUAL TO}9223372036854775807), M (1\N{LESS-THAN OR EQUAL TO}M\N{LESS-THAN OR EQUAL TO}100000) и K (0\N{LESS-THAN OR EQUAL TO}K\N{LESS-THAN OR EQUAL TO}100000)  — максимальное число, номер максимального множества и количество запросов к структуре данных. Далее следуют K строк указанного формата запросов.

Выходные данные
На каждый запрос LISTSET Ваша программа должна вывести числа  — содержимое запрошенного множества или \N{MINUS SIGN}1, если множество пусто.

На каждый запрос LISTSETSOF Ваша программа должна вывести числа  — номера множеств, содержащих запрошенное число, или \N{MINUS SIGN}1, если таких множеств не существует.

На прочие запросы не должно быть никакого вывода.

Гарантируется, что правильный вывод программы не превышает одного мегабайта.

Пример
входные данные
10 10
9
ADD 1 1
ADD 1 2
ADD 2 1
LISTSET 1
LISTSETSOF 1
DELETE 1 1
LISTSET 1
CLEAR 1
LISTSET 1
выходные данные
1 2
1 2
2
-1
Примечание
Эту задачу можно (и нужно!) решать, используя std::set и std::map.


*/

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

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

            lNode = lNode->unlinkChild(replacement->key);

            replacement->right() = rNode;
            replacement->left() = lNode;

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

        void dumpLine() {
            if (isStub()) return;

            left()->dumpLine();
            printf("%s ", std::to_string(key).c_str());
            right()->dumpLine();
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

        void dumpLine() {
            root->dumpLine();
            printf("\n");
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


    // AVL Map =========================================================================


    template <typename T, typename U> class AVLMap;  // Forward declaration for befriending

    template <typename T, typename U>
    class AVLMapNode {
        friend class AVLMap<T, U>;
    public:
        T key;
        U val;

        static AVLMapNode<T, U> *const STUB;

        static AVLMapNode<T, U> *create() {
            AVLMapNode<T, U> *self = new AVLMapNode<T, U>();
            assert(self);

            return self->ctor();
        }

        AVLMapNode<T, U> *ctor() {
            key = {};
            val = {};
            height = 0;
            children[0] = nullptr;
            children[1] = nullptr;

            return this;
        }

        AVLMapNode<T, U> *ctor(T new_key, U new_val) {
            key = new_key;
            val = new_val;
            height = 1;
            children[0] = STUB;
            children[1] = STUB;

            return this;
        }

        AVLMapNode<T, U> *ctor(T new_key, U new_val, AVLMapNode<T, U> *new_left, AVLMapNode<T, U> *new_right) {
            key = new_key;
            val = new_val;
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

        AVLMapNode<T, U> *edgeChild(bool isRight) {
            assert(!isStub());

            if (!children[isRight]->isStub())
                return children[isRight]->edgeChild(isRight);

            return this;
        }

        // Convenience wrappers
        AVLMapNode<T, U> *min() {
            return edgeChild(false);
        }

        AVLMapNode<T, U> *max() {
            return edgeChild(true);
        }

        AVLMapNode<T, U> *erase() {
            if (isStub()) return STUB;

            AVLMapNode<T, U> *lNode = left();
            AVLMapNode<T, U> *rNode = right();  // Kind of caching, because we'll delete 'this' in several cases
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

            AVLMapNode<T, U> *replacement = lNode->max();  // The rightmost child of the left subtree
            assert(!replacement->isStub());  // At least one node must exist there

            lNode = lNode->unlinkChild(replacement->key);

            replacement->right() = rNode;
            replacement->left() = lNode;

            delete this;

            replacement->updateHeight();

            return replacement->balance();
        }

        AVLMapNode<T, U> *erase(T goal_key) {
            if (isStub()) return STUB;

            if (key == goal_key) return erase();

            bool isRight = goal_key > key;

            children[isRight] = children[isRight]->erase(goal_key);

            updateHeight();

            return balance();
        }

        AVLMapNode<T, U> *insert(T new_key, U new_val) {
            //printf("> (%d)\n", key);

            if (isStub()) return AVLMapNode<T, U>::create()->ctor(new_key, new_val);

            if (new_key == key) {
                val = new_val;

                return this;
            }

            bool isRight = new_key > key;

            children[isRight] = children[isRight]->insert(new_key, new_val);

            updateHeight();

            return balance();
        }

        AVLMapNode<T, U> *find(T goal_key) {
            if (isStub()) return STUB;

            if (key == goal_key) return this;

            return children[goal_key > key]->find(goal_key);
        }

        AVLMapNode<T, U> *findClosest(T goal_key, bool isRight) {
            /*printf("> %p ", this);
            if (this) printf("(key=%d)", key);
            printf("\n");*/

            if (isStub()) return STUB;

            if (key == goal_key) {  // Maybe an unnecessary optimization, but why not
                return children[isRight]->edgeChild(!isRight);
            }

            AVLMapNode<T, U> *result = children[goal_key > key]->findClosest(goal_key, isRight);

            if (result->isStub() && (isRight == (goal_key < key))) return this;  // This essentially says that if the goal subtree is empty, the current node is the goal

            return result;
        }

        AVLMapNode<T, U> *minAbove(T goal_key) {
            return findClosest(goal_key, true);
        }

        AVLMapNode<T, U> *maxBelow(T goal_key) {
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
                printf(" (%s: %s):\n", std::to_string(key).c_str(), std::to_string(val).c_str());

                left ()->dump(depth + 1);
                right()->dump(depth + 1);
            }
        }
        #endif // ALGO_DEBUG

    #ifndef ALGO_DEBUG
    protected:
    #endif
        int height;
        AVLMapNode<T, U> *children[2];  // {left, right}

        static AVLMapNode<T, U> STUBVAL;

        // The following two methods are provided solely for convenience purposes as aliases
        // for children[0] and children[1]. The same effect could have been achieved with
        // #define left children[0] and #define right children[1], but I decided
        // this approach to be a better code practice
        inline AVLMapNode<T, U> *&left() {
            assert(!isStub());

            return children[0];
        }

        inline AVLMapNode<T, U> *&right() {
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
        AVLMapNode<T, U> *rotateSmall(bool isRight) {
            assert(!isStub());

            AVLMapNode<T, U> *otherNode = children[!isRight];
            assert(!otherNode->isStub());

            children[!isRight] = otherNode->children[isRight];

            otherNode->children[isRight] = this;

            updateHeight();
            otherNode->updateHeight();

            return otherNode;
        }

        // Same deal
        AVLMapNode<T, U> *rotateGrand(bool isRight) {
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

        AVLMapNode<T, U> *balance() {
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

        AVLMapNode<T, U> *unlinkChild(T goal_key) {
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


    template <typename T, typename U>
    class AVLMap {
    public:
        static AVLMap<T, U> *create() {
            AVLMap<T, U> *self = new AVLMap<T, U>();
            assert(self);

            return self->ctor();
        }

        AVLMap<T, U> *ctor() {
            root = AVLMapNode<T, U>::STUB;

            return this;
        }

        void dtor() {
            if (root && !root->isStub()) {
                root->dtor();
                free(root);
            }
        }

        AVLMapNode<T, U> *getRootSubtree() {
            // Whilst root as a field should probably be private, I can see people willing to make requests on subtrees,
            // so they probably have the right to obtain the root subtree as well
            return root;
        }

        void insert(T key, U value) {
            root = root->insert(key, value);
        }

        void erase(T key) {
            root = root->erase(key);
        }

        bool isEmpty() {
            return root->isStub();
        }

        AVLMapNode<T, U> *find(T key) {
            return root->find(key);
        }

        U get(T key) {
            AVLMapNode<T, U> *result = find(key);
            assert(!result->isStub());

            return result->val;
        }

        bool contains(T key) {
            return find(key) != AVLMapNode<T, U>::STUB;
        }

        #ifdef ALGO_DEBUG
        void dump() {
            printf("AVLMap [%p] {\n", this);
            root->dump(1);
            printf("}\n");
        }
        #endif // ALGO_DEBUG

    #ifndef ALGO_DEBUG
    protected:
    #endif
        AVLMapNode<T, U> *root;
    };
}


template <typename T>
abel::AVLNode<T> abel::AVLNode<T>::STUBVAL{};

template <typename T>
abel::AVLNode<T> *const abel::AVLNode<T>::STUB = &abel::AVLNode<T>::STUBVAL;

template <typename T, typename U>
abel::AVLMapNode<T, U> abel::AVLMapNode<T, U>::STUBVAL{};

template <typename T, typename U>
abel::AVLMapNode<T, U> *const abel::AVLMapNode<T, U>::STUB = &abel::AVLMapNode<T, U>::STUBVAL;


static const int CMD_LEN = 16;
static const char CMD_FMT[] = "%15s";

int main() {
    typedef unsigned long long Key;
    typedef abel::AVLTree<Key> Set;
    typedef abel::AVLTree<int> MetaSet;
    typedef abel::AVLMap<Key, MetaSet *> Map;

    int res = 0;

    Key n = 0;
    int m = 0, k = 0;
    res = scanf("%llu %d %d", &n, &m, &k);
    assert(res == 3);

    m++;

    Set *sets = (Set *)calloc(m, sizeof(Set));
    Map setsWithItem = {};
    setsWithItem.ctor();
    abel::AVLTree<MetaSet *> garbage = {};
    garbage.ctor();

    for (int i = 0; i < m; ++i) {
        sets[i].ctor();
    }

    char cmd[CMD_LEN] = "";

    for (int i = 0; i < k; ++i) {
        res = scanf(CMD_FMT, cmd);
        assert(res == 1);

        Key argItem = 0;
        int argSet = 0;

        #define CMD_CASE_(CMD, CODE) \
        if (strcmp(cmd, CMD) == 0) { \
            CODE \
        } else

        CMD_CASE_("ADD",
            res = scanf("%llu %d", &argItem, &argSet);
            assert(res == 2);

            sets[argSet].insert(argItem);

            if (!setsWithItem.contains(argItem)) {
                MetaSet *newSet = MetaSet::create();

                setsWithItem.insert(argItem, newSet);
                garbage.insert(newSet);
            }

            setsWithItem.get(argItem)->insert(argSet);
        )
        CMD_CASE_("DELETE",
            res = scanf("%llu %d", &argItem, &argSet);
            assert(res == 2);

            sets[argSet].erase(argItem);
            setsWithItem.get(argItem)->erase(argSet);
        )
        CMD_CASE_("CLEAR",
            res = scanf("%d", &argSet);
            assert(res == 1);

            while (!sets[argSet].isEmpty()) {
                Key tmp = sets[argSet].min();
                setsWithItem.get(tmp)->erase(argSet);
                sets[argSet].erase(tmp);
            }
        )
        CMD_CASE_("LISTSET",
            res = scanf("%d", &argSet);
            assert(res == 1);

            if (sets[argSet].isEmpty()) {
                printf("-1\n");
            } else {
                sets[argSet].dumpLine();
            }
        )
        CMD_CASE_("LISTSETSOF",
            res = scanf("%llu", &argItem);
            assert(res == 1);

            if (!setsWithItem.contains(argItem)) {
                printf("-1\n");
            } else {
                MetaSet *tmp = setsWithItem.get(argItem);

                if (tmp->isEmpty()) {
                    printf("-1\n");
                } else {
                    tmp->dumpLine();
                }
            }
        ) {
            assert(false);
            return 1;
        }

        #undef CMD_CASE_
    }


    for (int i = 0; i < m; ++i) {
        sets[i].dtor();
    }

    delete sets;

    while (!garbage.isEmpty()) {
        MetaSet *tmp = garbage.min();
        garbage.erase(tmp);
        tmp->dtor();
        delete tmp;
    }

    garbage.dtor();

    return 0;
}


/*

Complexity: O(K^2) in the worst case (additions to one set and then continuous list_set's
            But it shouldn't matter too much as this isn't a time-focused task

*/



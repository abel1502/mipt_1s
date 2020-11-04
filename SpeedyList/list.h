/**
 * @file list.h
 * A secure & fast list library
 *
 * ### Prerequisites
 *
 * To use, you should/may define the following:
 *
 * - [typedef] (required) list_elem_t @n
 * The type of elements on which the list operates
 *
 * - [define]  (optional) LIST_ELEM_FMT @n
 * Format string for list_elem_t (i.e. "%lg" for double, including the quotes)
 *
 * - [define]  (optional) LIST_VALIDATION_LEVEL @n
 * A number from 0 to 3, specifying, how much validation will be performed. Defaults to maximum.
 * 0 - no integrity checks; 1 - only the fast integrity checks; 2 - all integrity checks that don't affect the asymptotical time; 3 - all integrity checks.
 *
 * - [define]  (optional) LIST_DUMPNAME @n
 * If defined, specifies dump file name (NO EXTENTION!). Otherwise, a default name will be auto-generated based on current time whenever list_dump is called
 *
 * - ... <TODO>
 *
 * @warning Erroneous above-stated definitions may result in compilation or even runtime errors,
 *          so make sure to check them properly. The library author does not take responsibility
 *          for any errors caused by wrong definitions.
 *
 */


#ifndef LIST_H_GUARD
#define LIST_H_GUARD

#include <stdlib.h>
//#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "../libs/checksum.h"

#ifdef _WIN32
#include <windows.h>
#endif // _WIN32

//--------------------------------------------------------------------------------

#ifndef LIST_VALIDATION_LEVEL
#define LIST_VALIDATION_LEVEL 9
#endif

#if (0 > LIST_VALIDATION_LEVEL)
#undef LIST_VALIDATION_LEVEL
#define LIST_VALIDATION_LEVEL 0
#endif

#if (LIST_VALIDATION_LEVEL > 3)
#undef LIST_VALIDATION_LEVEL
#define LIST_VALIDATION_LEVEL 3
#endif

#if (LIST_VALIDATION_LEVEL >= 3)
//
#else
//
#endif


#if (LIST_VALIDATION_LEVEL >= 2)
#define LIST_USE_WIN_PTRCHECK 1
#else
#define LIST_USE_WIN_PTRCHECK 0
#endif

#if (LIST_VALIDATION_LEVEL >= 1)
#define LIST_USE_CANARY 1
#else
#define LIST_USE_CANARY 0
#endif

//--------------------------------------------------------------------------------

#ifndef MACROFUNC  // Same as in tests, but may not be included
#define MACROFUNC(...) do {__VA_ARGS__} while (0)
#endif


#if (LIST_VALIDATION_LEVEL > 0)

#define ASSERT_OK()  MACROFUNC(                                                        \
    if (list_validate(self) != LIST_VALID) {                                         \
        fprintf(stderr, "==============[ !!! CRITICAL FAILURE !!! ]==============\n"); \
        fprintf(stderr, "              (or not, but suck it anyway)              \n"); \
        fprintf(stderr, "[%s#%d]\n\n", __FILE__, __LINE__);                            \
        list_dump(self);                                                              \
        fprintf(stderr, "========================================================\n"); \
        abort();                                                                       \
    } )

#define REQUIRE(stmt)  MACROFUNC(                                                              \
    if (!(stmt)) {                                                                             \
        fprintf(stderr, "\nRequirement (%s) not met in (%s#%d)\n", #stmt, __FILE__, __LINE__); \
        abort();                                                                               \
    } )

#else

#define ASSERT_OK()  MACROFUNC()

#define REQUIRE(stmt)  MACROFUNC()

#endif

//--------------------------------------------------------------------------------

static const int LIST_HARD_CAP = (int)(((unsigned)-1) >> 3) / sizeof(list_elem_t);
static const char LIST_DUMP_FILE_FMT[] = "dump/list-dump-%y%m%d-%H%M%S";
static const int LIST_MAX_NODE_LABEL = 128;

#ifndef __cplusplus
typedef enum { false, true } bool;
#endif

#if LIST_USE_CANARY
typedef unsigned long long list_canary_t;
static const list_canary_t LIST_CANARY = ((list_canary_t)-1) & 0xBADC0DEDEADB1AD;  // BAD CODE DEAD BIRD
#endif

typedef struct list_s list_t;
typedef struct list_node_s list_node_t;
typedef int list_index_t;

/**
 * An enum describing the list's allocation state
 */
typedef enum {
    LAS_EXTERNAL,   ///< Allocated by the user's code
    LAS_HEAP,       ///< Allocated by the library's code on the heap
    LAS_FREED       ///< Already destroyed
} list_allocState_e;

/**
 * The list itself
 */
struct list_s {
    #if LIST_USE_CANARY
    list_canary_t leftCanary;  ///< Left canary
    #endif

    list_node_t *buf;          ///< Actual list nodes on the heap
    list_index_t size;         ///< List's current size
    list_index_t capacity;     ///< List's maximal size
    bool inArrayMode;          ///< Whether the list is in a sped-up mode for lookups
    list_allocState_e state;   ///< List allocation state

    list_index_t free;         ///< First free element's index

    #if LIST_USE_CANARY
    list_canary_t rightCanary; ///< Right canary
    #endif
};

/**
 * A node of the list
 */
struct list_node_s {
    list_elem_t value;  ///< Node's actual value
    list_index_t prev;  ///< Previous node (or -1 for free nodes)
    list_index_t next;  ///< Next node
};

/**
 * List validity info
 */
typedef enum {
    LIST_VALID = 0,  ///< Valid
    LIST_BADPTR,     ///< Some pointer is corrupt
    LIST_BADSIZE,    ///< Size or capacity is corrupt
    LIST_BADCANARY,  ///< A canary is corrupt
    LIST_LOOP,       ///< The list contains a loop
    LIST_UAF         ///< Use after free
} list_validity_e;


/**
 * List constructor (internal allocation)
 *
 * @param [in]  capacity  Initial capacity
 *
 * @return List instance, NULL on error
 */
list_t *list_new(list_index_t capacity);

/**
 * List constructor (external allocation)
 *
 * @param [in/out] self      List instance
 * @param [in]     capacity  Initial capacity
 *
 * @return `self`, NULL on error
 */
list_t *list_init(list_t *self, list_index_t capacity);

/**
 * List destructor (internal allocation)
 *
 * @param [in/out] self  List instance
 */
void list_destroy(list_t *self);

/**
 * List destructor (external allocation)
 *
 * @param [in/out] self  List instance
 */
void list_free(list_t *self);

/**
 * Insert `value` to the list
 *
 * @param [in/out] self   List instance
 * @param [in]     node   The node before which to insert
 * @param [in]     value  The value to insert
 *
 * @return true on error, false otherwise
 */
bool list_insertBefore(list_t *self, list_index_t node, list_elem_t value);

/**
 * Insert `value` to the list
 *
 * @param [in/out] self   List instance
 * @param [in]     node   The node after which to insert
 * @param [in]     value  The value to insert
 *
 * @return true on error, false otherwise
 */
bool list_insertAfter(list_t *self, list_index_t node, list_elem_t value);

/**
 * Remove `node` from the list
 *
 * @param [in/out] self   List instance
 * @param [in]     node   The node to remove
 *
 * @return true on error, false otherwise
 */
bool list_remove(list_t *self, list_index_t node);

/**
 * Find a list node by its list-index
 *
 * @warning Works in linear time, unless self.inArrayMode!
 * @warning Do not confuse list-index with node-index: the latter may used to directly access a node in the buf
 *
 * @param [in/out] self   List instance
 * @param [in]     ind    The index to look up
 * @param [out]    value  The destination for the searched node's value
 *
 * @return true on error, false otherwise
 */
bool list_findByIndex(const list_t *self, int ind, list_elem_t *value);

/**
 * Transforms a node-index into a node pointer
 * (mostly for internal use)
 *
 * @param [in/out] self   List instance
 * @param [in]     node   The node-index
 *
 * @return &self->buf[node] or NULL if index is out of range
 */
list_node_t *list_getNode(const list_t *self, list_index_t node);

/**
 * Test if a node is free
 *
 * @param [in/out] self   List instance
 * @param [in]     node   The node to check
 *
 * @return true if node is free, false otherwise
 */
bool list_isNodeFree(const list_t *self, list_index_t node);

/**
 * Mark a node as free
 *
 * @param [in/out] self   List instance
 * @param [in]     node   The node to mark
 *
 * @return true on error, false otherwise
 */
bool list_setNodeFree(list_t *self, list_index_t node);

/**
 * Transforms the list in such a way that lookups by index now take O(1) time
 *
 * @warning Works in O(n log n)
 * @warning Highly volatile, will automatically cancel upon any insertion/deletion, except head ones
 */
bool list_enterArrayMode(list_t *self);

/**
 * Resize the list
 *
 * @param [in/out] self      List instance
 * @param [in]     capacity  New desired capacity (>= capacity)
 *
 * @return true on error, false otherwise
 */
bool list_resize(list_t *self, list_index_t capacity);

/**
 * Clear the list
 *
 * @param [in/out] self  List instance
 */
void list_clear(list_t *self);

/**
 * Is list empty?
 *
 * @param [in]  self  List instance
 *
 * @return true if self is empty, false otherwise
 */
bool list_isEmpty(const list_t *self);

static int list_nextFreeCell(list_t *self);

static void list_dumpInfoBox(const list_t *self, FILE *dumpFile);

static void list_dumpNode(const list_t *self, list_index_t node, FILE *dumpFile);

static void list_dumpGraph(const list_t *self, FILE *dumpFile);

/**
 * Dump the list (for debug)
 *
 * @param [in]  self  List instance
 */
void list_dump(const list_t *self);

/**
 * Validate the list
 *
 * @param [in]  self  List instance
 *
 * @return List's validity
 */
list_validity_e list_validate(const list_t *self);

/**
 * Retrieve a readable description for list_validity
 *
 * @param [in]  self  List validity value
 *
 * @return Readable description
 */
const char *list_validity_describe(list_validity_e self);

/**
 * Retrieve a readable description for list_allocState
 *
 * @param [in]  self  List allocState value
 *
 * @return Readable description
 */
const char *list_allocState_describe(list_allocState_e self);

#ifndef LIST_NOIMPL

#if LIST_USE_CANARY
/**
 * Address of list buf's left canary
 *
 * @param [in]  self  List instance
 *
 * @return Left canary address
 */
static list_canary_t *list_leftBufCanary(const list_t *self);

/**
 * Address of list buf's right canary
 *
 * @param [in]  self  List instance
 *
 * @return Right canary address
 */
static list_canary_t *list_rightBufCanary(const list_t *self);
#endif // LIST_USE_CANARY

static char *genDumpFileName();

#endif // LIST_NOIMPL

/**
 * Check if ptr is a valid pointer
 *
 * @param [in]  ptr  The pointer to be checked
 *
 * @return true if `ptr` is valid, false otherwise
 */
bool isPointerValid(const void *ptr);

#ifdef TEST
void test_list(list_elem_t val1, list_elem_t val2, list_elem_t val3);
#endif

//--------------------------------------------------------------------------------

#ifndef LIST_NOIMPL

list_t *list_new(list_index_t capacity) {
    list_t *self = (list_t *)calloc(1, sizeof(list_t));

    if (!isPointerValid(self)) {
        return NULL;
    }

    if (list_init(self, capacity) == NULL) {
        return NULL;
    }

    self->state = LAS_HEAP;

    ASSERT_OK();

    return self;
}

list_t *list_init(list_t *self, list_index_t capacity) {
    if (!isPointerValid(self) || capacity == 0 || capacity >= LIST_HARD_CAP) {
        return NULL;
    }

    #if LIST_USE_CANARY
    self->leftCanary  = LIST_CANARY;
    self->rightCanary = LIST_CANARY;
    #endif

    self->capacity = capacity;
    self->size = 0;
    self->state = LAS_EXTERNAL;
    self->free = 1;
    self->inArrayMode = true;

    #if LIST_USE_CANARY
    self->buf = (list_node_t *)calloc(1, (1 + capacity) * sizeof(list_node_t) + sizeof(list_canary_t) * 2);
    #else
    self->buf = (list_node_t *)calloc(1 + capacity, sizeof(list_node_t));
    #endif

    if (!isPointerValid(self->buf)) {
        return NULL;
    }

    #if LIST_USE_CANARY
    self->buf = (list_node_t *)((list_canary_t *)self->buf + 1);

    *list_leftBufCanary(self)  = LIST_CANARY;
    *list_rightBufCanary(self) = LIST_CANARY;
    #endif

    list_clear(self);

    ASSERT_OK();

    return self;
}

void list_destroy(list_t *self) {
    ASSERT_OK();

    REQUIRE(self->state == LAS_HEAP);

    self->state = LAS_EXTERNAL;

    list_free(self);

    free(self);
}

void list_free(list_t *self) {
    ASSERT_OK();

    REQUIRE(self->state == LAS_EXTERNAL);

    #if LIST_USE_CANARY
    free(list_leftBufCanary(self));

    self->leftCanary = 0;
    self->rightCanary = 0;
    #else
    free(self->buf);
    #endif

    self->buf = NULL;
    self->size = 0;
    self->capacity = 0;
    self->state = LAS_FREED;
    self->free = 0;
    self->inArrayMode = false;
}

#define LIST_INSERT_TPL_(RLNODE_INIT_CODE) \
    ASSERT_OK(); \
    \
    if (self->size >= self->capacity) { \
        if (list_resize(self, self->capacity * 2)) { \
            return true; \
        } \
    } \
    \
    RLNODE_INIT_CODE \
    \
    list_index_t cur = list_nextFreeCell(self); \
    \
    list_node_t *curNode = list_getNode(self, cur); \
    REQUIRE(curNode != NULL); \
    \
    if (oldLNode->next != 0) { \
        self->inArrayMode = false; \
    } \
    \
    curNode->value = value; \
    \
    curNode->next = oldLNode->next; \
    curNode->prev = oldRNode->prev; \
    oldLNode->next = cur; \
    oldRNode->prev = cur; \
    \
    self->size++; \
    \
    ASSERT_OK(); \
    \
    return false;

bool list_insertBefore(list_t *self, list_index_t node, list_elem_t value) {
    LIST_INSERT_TPL_(
        list_node_t *oldRNode = list_getNode(self, node);
        if (oldRNode == NULL || oldRNode->prev == -1) {
            return true;
        }

        list_node_t *oldLNode = list_getNode(self, oldRNode->prev);
        REQUIRE(oldLNode != NULL);
    )
}

bool list_insertAfter(list_t *self, list_index_t node, list_elem_t value) {
    LIST_INSERT_TPL_(
        list_node_t *oldLNode = list_getNode(self, node);
        if (oldLNode == NULL || oldLNode->prev == -1) {
            return true;
        }

        list_node_t *oldRNode = list_getNode(self, oldLNode->next);
        REQUIRE(oldRNode != NULL);
    )
}

#undef LIST_INSERT_TPL_

bool list_remove(list_t *self, list_index_t node) {
    ASSERT_OK();

    list_node_t *curNode = list_getNode(self, node);
    if (curNode == NULL) {
        return true;
    }

    list_node_t *lNode = list_getNode(self, curNode->prev);
    REQUIRE(lNode != NULL);

    list_node_t *rNode = list_getNode(self, curNode->next);
    REQUIRE(rNode != NULL);

    lNode->next = curNode->next;
    rNode->prev = curNode->prev;

    if (list_setNodeFree(self, node)) {
        return true;
    }

    self->size--;

    ASSERT_OK();

    return false;
}

bool list_findByIndex(const list_t *self, int ind, list_elem_t *value) {
    ind = (ind % self->size + self->size) % self->size + 1;

    list_node_t *curNode = NULL;

    if (self->inArrayMode) {
        curNode = list_getNode(self, ind);
        REQUIRE(curNode != NULL);
    } else {
        curNode = list_getNode(self, 0);
        REQUIRE(curNode != NULL);

        for (int i = 0; i < ind; ++i) {
            curNode = list_getNode(self, curNode->next);
            REQUIRE(curNode != NULL);
        }
    }

    if (curNode->prev == -1) {  // Free node
        return true;
    }

    *value = curNode->value;

    return false;
}

list_node_t *list_getNode(const list_t *self, list_index_t node) {
    ASSERT_OK();

    if (node < 0 || node > self->capacity) return NULL;

    return self->buf[node].prev == -1;
}

bool list_isNodeFree(const list_t *self, list_index_t node) {
    ASSERT_OK();

    return list_getNode(self, node)->prev == -1;
}

bool list_setNodeFree(list_t *self, list_index_t node) {
    ASSERT_OK();

    list_node_t *curNode = list_getNode(self, node);

    if (curNode == NULL) {
        return true;
    }

    curNode->next = self->free;
    curNode->prev = -1;
    self->free = node;

    return false;
}

bool list_enterArrayMode(list_t *self);

bool list_resize(list_t *self, list_index_t capacity) {
    ASSERT_OK();

    if (capacity < self->capacity || capacity >= LIST_HARD_CAP) {
        return true;
    }

    if (self->buf[self->capacity].prev == -1) {
        self->buf[self->capacity].next = self->capacity + 1;
    } else {
        self->free = self->capacity + 1;
    }

    int oldCap = self->capacity;

    #if LIST_USE_CANARY
    list_node_t *newBuf = (list_node_t *)realloc(list_leftBufCanary(self), (1 + capacity) * sizeof(list_node_t) + 2 * sizeof(list_canary_t));
    #else
    list_node_t *newBuf = (list_node_t *)realloc(self->buf, (1 + capacity) * sizeof(list_node_t));
    #endif

    if (!isPointerValid(newBuf)) {
        return true;
    }

    #if LIST_USE_CANARY
    self->buf = (list_node_t *)((list_canary_t *)newBuf + 1);
    #else
    self->buf = newData;
    #endif

    self->capacity = capacity;

    #if LIST_USE_CANARY
    *list_leftBufCanary(self)  = LIST_CANARY;
    *list_rightBufCanary(self) = LIST_CANARY;
    #endif

    for (list_index_t i = 1; i < self->capacity; ++i) {
        self->buf[i].next = i + 1;
        self->buf[i].prev = -1;
    }

    self->buf[self->capacity].next = self->free;
    self->buf[self->capacity].prev = -1;

    ASSERT_OK();

    return false;
}

void list_clear(list_t *self) {
    // TODO: clear values too?
    self->buf[0].next = 0;
    self->buf[0].prev = 0;

    for (int i = 1; i < self->capacity; ++i) {
        self->buf[i].next = i + 1;
        self->buf[i].prev = -1;
    }

    self->buf[self->capacity].next = 1;
    self->buf[self->capacity].prev = -1;
}

bool list_isEmpty(const list_t *self) {
    ASSERT_OK();

    return self->size == 0;
}

static int list_nextFreeCell(list_t *self) {
    ASSERT_OK();  // TODO: Remove?

    REQUIRE(self->free != 0);

    int nextFree = self->free;

    self->free = self->buf[self->free].next;

    ASSERT_OK();

    return nextFree;
}

#define DUMP_(...)  fprintf(dumpFile, ##__VA_ARGS__)
#define EOL_  "<br align=\"left\"/>"

static void list_dumpInfoBox(const list_t *self, FILE *dumpFile) {
    DUMP_("info [\n"
          "shape=note\n"
          "fontname=Consolas\n"
          "margin=0.1\n");

    DUMP_("label=<\n");


    #ifdef LIST_ELEM_FMT
    DUMP_("<font color=\"crimson\">[WARNING: LIST_ELEM_FMT is specified, so the dump may fail through the user\'s fault.]  </font>" EOL_);
    #endif

    #define CANARY_FMT_       "<font color=\"%s\">0x%016llX</font>"
    #define CANARY_ARG_(VAL)  (VAL) == LIST_CANARY ? "green" : "crimson", (VAL)

    list_validity_e validity = list_validate(self);
    DUMP_("list_t (<font color=\"%s\">%s</font>) [0x%p] {" EOL_, validity == LIST_VALID ? "green" : "crimson", list_validity_describe(validity), self);
    if (isPointerValid(self)) {
        #if LIST_USE_CANARY
        DUMP_("   left canary     = " CANARY_FMT_ EOL_, CANARY_ARG_(self->leftCanary));
        #endif

        DUMP_("   size            = %d" EOL_, self->size);
        DUMP_("   capacity        = %d (out of %d)" EOL_, self->capacity, LIST_HARD_CAP);
        DUMP_("   free            = #%d" EOL_, self->free);
        DUMP_("   state           = %s" EOL_, list_allocState_describe(self->state));
        DUMP_("   inArrayMode     = %s" EOL_, self->inArrayMode ? "true" : "false");

        DUMP_("   buf [0x%p] {"EOL_, self->buf);
        if (isPointerValid(self->buf)) {
            #if LIST_USE_CANARY
            list_canary_t lCan = *list_leftBufCanary(self);
            DUMP_("     l. canary     = " CANARY_FMT_ EOL_, CANARY_ARG_(lCan));
            #endif

            DUMP_("     ..." EOL_);

            #if LIST_USE_CANARY
            list_canary_t rCan = *list_rightBufCanary(self);
            DUMP_("     r. canary     = " CANARY_FMT_ EOL_, CANARY_ARG_(rCan));
            #endif
        } else {
            DUMP_("     <corrupt>" EOL_);
        }
        DUMP_("   }"EOL_);

        #if LIST_USE_CANARY
        DUMP_("   right canary    = " CANARY_FMT_ EOL_, CANARY_ARG_(self->rightCanary));
        #endif
    } else {
        DUMP_("   <corrupt>" EOL_);
    }
    DUMP_("}" EOL_);

    #undef CANARY_FMT_
    #undef CANARY_ARG_

    DUMP_(">\n");

    DUMP_("]\n");
}

static void list_dumpNode(const list_t *self, list_index_t node, FILE *dumpFile) {
    REQUIRE(self != NULL);

    list_node_t *nodePtr = list_getNode(self, node);

    const char *fillColor = "white";
    char label[LIST_MAX_NODE_LABEL + 1] = "";

    if (nodePtr == NULL) {
        fillColor = "crimson";

        strcpy(label, "<b>NULL</b>");
    } else if (list_isNodeFree(self, node)) {
        fillColor = "limegreen";

        #ifdef LIST_ELEM_FMT
        int res = snprintf(label, LIST_MAX_NODE_LABEL + 1, "[%d] <b>" LIST_ELEM_FMT "</b>", node, nodePtr->value);
        #else
        int res = snprintf(label, LIST_MAX_NODE_LABEL + 1, "[%d]", node);
        #endif
        REQUIRE(0 <= res && res <= LIST_MAX_NODE_LABEL);

        DUMP_("%d -> %d [color=green]\n", node, nodePtr->next);
    } else if (node == 0) {
        fillColor = "#FF6900";

        #ifdef LIST_ELEM_FMT
        int res = snprintf(label, LIST_MAX_NODE_LABEL + 1, "[0] <b>" LIST_ELEM_FMT "</b>", nodePtr->value);
        #else
        int res = snprintf(label, LIST_MAX_NODE_LABEL + 1, "[0]");
        #endif
        REQUIRE(0 <= res && res <= LIST_MAX_NODE_LABEL);

        DUMP_("%d -> %d [color=blue]\n", nodePtr->prev, node);
        DUMP_("%d -> %d [color=red]\n", node, nodePtr->next);
    } else {
        fillColor = "#0096FF";

        #ifdef LIST_ELEM_FMT
        int res = snprintf(label, LIST_MAX_NODE_LABEL + 1, "[%d] <b>" LIST_ELEM_FMT "</b>", node, nodePtr->value);
        #else
        int res = snprintf(label, LIST_MAX_NODE_LABEL + 1, "[%d]", node);
        #endif
        REQUIRE(0 <= res && res <= LIST_MAX_NODE_LABEL);

        DUMP_("%d -> %d [color=blue]\n", nodePtr->prev, node);
        DUMP_("%d -> %d [color=red]\n", node, nodePtr->next);
    }

    DUMP_("%d [fontname=Consolas margin=\"0.12,0\" shape=box style=filled color=black fillcolor=\"%s\" label=<%s>]\n", node, fillColor, label);
}

static void list_dumpGraph(const list_t *self, FILE *dumpFile) {
    REQUIRE(dumpFile != NULL);

    DUMP_("digraph list_t {\n");

    DUMP_("graph [rankdir=LR splines=true pack=true]\n");

    list_dumpInfoBox(self, dumpFile);

    DUMP_("subgraph clusterBuf {\n");
    if (self == NULL || self->buf == NULL) {
        DUMP_("\"<corrupt>\" [shape=box style=filled color=crimson]\n");
    } else {
        for (list_index_t node = 0; node <= self->capacity; ++node) {
            list_dumpNode(self, node, dumpFile);
        }
    }
    DUMP_("}\n");

    DUMP_("}\n");
}

#undef EOL_
#undef DUMP_

void list_dump(const list_t *self) {
    char *dumpFileName = genDumpFileName();

    FILE *dumpFile = fopen(dumpFileName, "wb");

    list_dumpGraph(self, dumpFile);

    fclose(dumpFile);

    char cmd[256] = "";

    sprintf(cmd, "dot -O -Tsvg %.100s", dumpFileName);
    system(cmd);
    //printf(">> %s\n", cmd);
    sprintf(cmd, "start %.100s.svg", dumpFileName);
    system(cmd);
    //printf(">> %s\n", cmd);

    free(dumpFileName);
}

list_validity_e list_validate(const list_t *self) {
    if (!isPointerValid(self) || !isPointerValid(self->buf)) {
        return LIST_BADPTR;
    }

    if (self->capacity == 0 || self->size > self->capacity || \
        self->capacity >= LIST_HARD_CAP || !isPointerValid(self->buf + self->capacity)) {
        return LIST_BADSIZE;
    }

    if (self->state == LAS_FREED) {
        return LIST_UAF;
    }

    #if LIST_USE_CANARY
    if (self->leftCanary != LIST_CANARY || self->rightCanary != LIST_CANARY || \
        !isPointerValid(list_leftBufCanary(self)) || !isPointerValid(list_rightBufCanary(self)) || \
        *list_leftBufCanary(self) != LIST_CANARY || *list_rightBufCanary(self) != LIST_CANARY) {
        return LIST_BADCANARY;
    }
    #endif

    // TODO: more validator

    return LIST_VALID;
}

#define DESCRIBE_(value, descr)  case value: return "&lt;" #value "&gt; " descr;
const char *list_validity_describe(list_validity_e self) {
    switch (self) {
        case LIST_VALID: return "ok";
        DESCRIBE_(LIST_BADPTR,    "Bad pointer")
        DESCRIBE_(LIST_BADSIZE,   "Bad size")
        DESCRIBE_(LIST_BADCANARY, "Bad canary")
        DESCRIBE_(LIST_LOOP,      "Loop")
        DESCRIBE_(LIST_UAF,       "Use after free")

        default: return "!CORRUPT VALIDITY!";
    }
}

const char *list_allocState_describe(list_allocState_e self) {
    switch (self) {
        DESCRIBE_(LAS_EXTERNAL, "external")
        DESCRIBE_(LAS_HEAP,     "heap")
        DESCRIBE_(LAS_FREED,    "freed")

        default: return "!CORRUPT ALLOCSTATE!";
    }
}
#undef DESCRIBE_

#if LIST_USE_CANARY
static list_canary_t *list_leftBufCanary(const list_t *self) {
    REQUIRE(isPointerValid(self));
    REQUIRE(isPointerValid(self->buf));

    return (list_canary_t *)self->buf - 1;
}

static list_canary_t *list_rightBufCanary(const list_t *self) {
    REQUIRE(isPointerValid(self));
    REQUIRE(isPointerValid(self->buf));

    return (list_canary_t *)(self->buf + self->capacity + 1);
}
#endif

bool isPointerValid(const void *ptr) {
    if (ptr < (const void *)4096) {
        // || ((size_t)ptr >> (sizeof(ptr) >= 8 ? 42 : 26)) != 0;
        // On Linux, sometimes the valid memory may occupy the highest possible addresses,
        // so we can't afford this heuristic check

        return false;
    }

    // I won't include a unix-specific check, because the only one I could find is extremely
    // inefficient and requires a non-const pointer

    #if defined(_WIN32) && LIST_USE_WIN_PTRCHECK
    // This one actually turned out to be incredibly slow, so I'll only include it at validation level 2, I guess

    // (c) Ded32, TXLib
    MEMORY_BASIC_INFORMATION mbi = {};
    if (!VirtualQuery(ptr, &mbi, sizeof (mbi)))
        return false;

    if (mbi.Protect & (PAGE_GUARD | PAGE_NOACCESS))
        return false;  // Guard page -> bad ptr

    DWORD readRights = PAGE_READONLY | PAGE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY;

    return (mbi.Protect & readRights) != 0;

    #endif // defined(_WIN32) && LIST_USE_WIN_PTRCHECK

    return true;
}

static char *genDumpFileName() {
    char *name = (char *)calloc(100, sizeof(char));
    REQUIRE(name != NULL);

    #ifdef LIST_DUMPNAME
    #define Q0_(x) Q1_(x)
    #define Q1_(x) #x
    strcpy(name, "dump/" Q0_(LIST_DUMPNAME));
    #undef Q1_
    #undef Q0_
    #else
    time_t rawtime;
    struct tm *timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    //strftime(name, 100, LIST_DUMP_FILE_FMT, timeinfo);
    REQUIRE(strftime(name, 100, LIST_DUMP_FILE_FMT, timeinfo) != 0);  // We'll trust it's enough
    #endif // LIST_DUMPNAME

    return name;
}

#ifdef TEST
void test_list(list_elem_t val1, list_elem_t val2, list_elem_t val3) {
    list_t *lst = list_new(8);

    TEST_ASSERT(lst != NULL);

    TEST_ASSERT(!list_insertBefore(lst, 0, val1));
    TEST_ASSERT(!list_insertBefore(lst, 0, val2));
    TEST_ASSERT(!list_insertAfter(lst, 0, val2));

    //list_dump(lst);
    //system("pause");

    TEST_ASSERT(!list_remove(lst, 1));

    //list_dump(lst);

    list_destroy(lst);
}
#endif

#endif // LIST_NOIMPL

#undef ASSERT_OK
#undef REQUIRE

#undef LIST_USE_CANARY

#undef LIST_VALIDATION_LEVEL
#undef LIST_ELEM_FMT
#undef LIST_DUMPNAME

#endif // LIST_H_GUARD

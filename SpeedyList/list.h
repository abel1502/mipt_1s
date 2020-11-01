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
 * - [define]  (optional) LIST_ELEM_PRINT @n
 * Name of the helper function to print list elements during debug dump.
 * Signature: void LIST_ELEM_PRINT(list_elem_t). The dump prints raw bytes in hex alongside, so this may be left undefined.
 *
 * - [define]  (optional) LIST_VALIDATION_LEVEL @n
 * A number from 0 to 3, specifying, how much validation will be performed. Defaults to maximum.
 * 0 - no integrity checks; 1 - only the fast integrity checks; 2 - all integrity checks that don't affect the asymptotical time; 3 - all integrity checks.
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
//
#else
//
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

static const size_t LIST_DUMP_LIMIT = 100;
static const size_t LIST_HARD_CAP = (((size_t)-1) >> 8) / sizeof(list_elem_t);

#ifndef __cplusplus
typedef enum { false, true } bool;
#endif

#if LIST_USE_CANARY
typedef unsigned long long list_canary_t;
static const list_canary_t LIST_CANARY = ((list_canary_t)-1) & 0xBADC0DEDEADB1AD;  // BAD CODE DEAD BIRD
#endif

typedef struct list_s list_t;
typedef struct list_node_s list_node_t;

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
    size_t size;               ///< List's current size
    size_t capacity;           ///< List's maximal size
    bool inArrayMode;          ///< Whether the list is in a sped-up mode for lookups
    list_allocState_e state;   ///< List allocation state

    size_t head;               ///< Head index
    size_t tail;               ///< Tail index
    size_t free;               ///< First free element's index

    #if LIST_USE_CANARY
    list_canary_t rightCanary; ///< Right canary
    #endif
};

/**
 * A node of the list
 */
struct list_node_s {
    list_elem_t value;
    size_t prev;
    size_t next;
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
list_t *list_new(size_t capacity);

/**
 * List constructor (external allocation)
 *
 * @param [in/out] self      List instance
 * @param [in]     capacity  Initial capacity
 *
 * @return `self`, NULL on error
 */
list_t *list_init(list_t *self, size_t capacity);

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
bool list_insertBefore(list_t *self, list_node_t *node, list_elem_t value);

/**
 * Insert `value` to the list
 *
 * @param [in/out] self   List instance
 * @param [in]     node   The node after which to insert
 * @param [in]     value  The value to insert
 *
 * @return true on error, false otherwise
 */
bool list_insertAfter(list_t *self, list_node_t *node, list_elem_t value);

/**
 * Remove `node` from the list
 *
 * @param [in/out] self   List instance
 * @param [in]     node   The node to remove
 *
 * @return true on error, false otherwise
 */
bool list_remove(list_t *self, list_node_t *node);

/**
 * Find a list node by its (list) index
 *
 * @warning Works in linear time, unless self.inArrayMode!
 *
 * @param [in/out] self   List instance
 * @param [in]     ind    The index to look up
 * @param [out]    value  The destination for the searched node's value
 *
 * @return true on error, false otherwise
 */
bool list_findByIndex(list_t *self, size_t ind, list_elem_t *value);

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
bool list_resize(list_t *self, size_t capacity);

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



#if LIST_USE_CANARY && !defined(LIST_NOIMPL)
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
#endif

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

// TODO

#endif // STACK_NOIMPL

#undef ASSERT_OK
#undef REQUIRE

#undef LIST_USE_CANARY

#undef LIST_VALIDATION_LEVEL
#undef LIST_ELEM_PRINT

#endif // LIST_H_GUARD

/**
 * @file stack.h
 * A secure stack library
 *
 * ### Prerequisites
 *
 * To use, you should/may define the following:
 *
 * - [typedef] (required) stack_elem_t @n
 * The type of elements on which the stack works
 *
 * - [define]  (optional) STACK_ELEM_PRINT @n
 * Name of the helper function to print stack elements during debug dump.
 * Signature: void STACK_ELEM_PRINT(stack_elem_t). The dump prints raw bytes in hex alongside, so this may be left undefined.
 *
 * - [define]  (optional) STACK_VALIDATION_LEVEL @n
 * A number from 0 to 2, specifying, how much validation will be performed. Defaults to maximum.
 * 0 - no integrity checks; 1 - only the fast integrity checks; 2 - all integrity checks.
 *
 * - ... <TODO>
 *
 * @warning Erroneous above-stated definitions may result in compilation or even runtime errors,
 *          so make sure to check them properly. The library author does not take responsibility
 *          for any errors caused by wrong definitions.
 *
 */


/*
    =====[ Global TODO ]=====
 # Write a basic stack
 # Protect the stack as hell)
 # Define ASSERT_OK
 ? Remove asserts from destructors
 # Change peek and pop to output pointer instead of rval?
 # Add error codes
 # Rework destructor SAS checks
 ? Macro-based overloads
 # Add release-time checks (+in constructors)
 # Make heavy debug checks conditionally compile
 # Make ASSERT_OK a functional-style marco
 # Poison
 # Canaries
 # Data Canaries
 # Hashes
 # isPointerValid
 # CRC32 lib
 # Unit tests
 ? Rework stack_clear not to rely on stack_pop
 # Documentation
 # Separate defines on top that say which features to turn on and are based on STACK_VALIDATION_LEVEL
 ? Endif comments
 # No FF in canary
 # Rename userspace
 # "Prerequisites" in docs
 ? Disable alphabetic sort in doxygen
 # Brackets around #if conds
 # Rename ASSERT to REQUIRE
 # Improved pointer validity check (find in TXLib)
 # Enum bool
 # Handle size_t overflow (in resize)
 # Maximal limit in dump - global const
 ? Macros to replace #if (STACK_USE_***)
 # Downward resize in pop
 # Another validation level to perform everything but data hashing
 - Debug console
 # Hard cap on capacity
 - Fancy logging
 ...
    =========================
*/


#ifndef STACK_H_GUARD
#define STACK_H_GUARD

#include <stdlib.h>
//#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../libs/checksum.h"

#ifdef _WIN32
#include <windows.h>
#endif // _WIN32

//--------------------------------------------------------------------------------

#ifndef STACK_VALIDATION_LEVEL
#define STACK_VALIDATION_LEVEL 9
#endif

#if (0 > STACK_VALIDATION_LEVEL)
#undef STACK_VALIDATION_LEVEL
#define STACK_VALIDATION_LEVEL 0
#endif

#if (STACK_VALIDATION_LEVEL > 3)
#undef STACK_VALIDATION_LEVEL
#define STACK_VALIDATION_LEVEL 3
#endif

#if (STACK_VALIDATION_LEVEL >= 3)
#define STACK_USE_HASH_DATA 1
#else
#define STACK_USE_HASH_DATA 0
#endif


#if (STACK_VALIDATION_LEVEL >= 2)
#define STACK_USE_HASH_STRUCT 1
#else
#define STACK_USE_HASH_STRUCT 0
#endif

#if (STACK_VALIDATION_LEVEL >= 1)
#define STACK_USE_POISON 1
#define STACK_USE_CANARY 1
#else
#define STACK_USE_POISON 0
#define STACK_USE_CANARY 0
#endif

//--------------------------------------------------------------------------------

#ifndef MACROFUNC  // Same as in tests, but may not be included
#define MACROFUNC(...) do {__VA_ARGS__} while (0)
#endif


#if (STACK_VALIDATION_LEVEL > 0)

#define ASSERT_OK()  MACROFUNC(                                                        \
    if (stack_validate(self) != STACK_VALID) {                                         \
        fprintf(stderr, "==============[ !!! CRITICAL FAILURE !!! ]==============\n"); \
        fprintf(stderr, "              (or not, but suck it anyway)              \n"); \
        fprintf(stderr, "[%s#%d]\n\n", __FILE__, __LINE__);                            \
        stack_dump(self);                                                              \
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

static const size_t STACK_DUMP_LIMIT = 100;
static const size_t STACK_HARD_CAP = (((size_t)-1) >> 8) / sizeof(stack_elem_t);

#ifndef __cplusplus
typedef enum { false, true } bool;
#endif

#if STACK_USE_POISON
static const unsigned char POISON = 0xAA;
#endif

#if STACK_USE_CANARY
typedef unsigned long long canary_t;
//static const canary_t CANARY = ((canary_t)-1 << 32) | 0xDEADB1AD;  // DEADBIRD
static const canary_t CANARY = ((canary_t)-1) & 0xBADC0DEDEADB1AD;  // BAD CODE DEAD BIRD
#endif

typedef struct stack_s stack_t;

/**
 * An enum describing the stack's allocation state
 */
typedef enum {
    SAS_EXTERNAL,   ///< Allocated by the user's code
    SAS_HEAP,       ///< Allocated by the library's code on the heap
    SAS_FREED       ///< Already destroyed
} stack_allocState_e;

/**
 * A stack of `stack_item_t`'s
 */
struct stack_s {
    #if STACK_USE_CANARY
    canary_t leftCanary;       ///< Left canary
    #endif

    stack_elem_t *data;        ///< Actual stack elements on the heap
    size_t size;               ///< Stack's current size
    size_t capacity;           ///< Stack's maximal size

    #if STACK_USE_HASH_STRUCT
    crc32_t structChecksum;    ///< A hash of this struct
    #endif

    #if STACK_USE_HASH_DATA
    crc32_t dataChecksum;      ///< A hash of the stack's data
    #endif

    stack_allocState_e state;  ///< Stack allocation state

    #if STACK_USE_CANARY
    canary_t rightCanary;      ///< Right canary
    #endif
};

/**
 * Stack validity info
 */
typedef enum {
    STACK_VALID = 0,  ///< Valid
    STACK_BADPTR,     ///< Some pointer is corrupt
    STACK_BADSIZE,    ///< Size or capacity is corrupt
    STACK_BADCANARY,  ///< A canary is corrupt
    STACK_BADHASH,    ///< A hash is corrupt
    STACK_BADPOISON,  ///< Poison is corrupt
    STACK_UAF         ///< Use after free
} stack_validity_e;


/**
 * Stack constructor (internal allocation)
 *
 * @param [in]  capacity  Initial capacity
 *
 * @return Stack instance, NULL on error
 */
stack_t *stack_new(size_t capacity);

/**
 * Stack constructor (external allocation)
 *
 * @param [in/out] self      Stack instance
 * @param [in]     capacity  Initial capacity
 *
 * @return `self`, NULL on error
 */
stack_t *stack_construct(stack_t *self, size_t capacity);

/**
 * Stack destructor (internal allocation)
 *
 * @param [in/out] self  Stack instance
 */
void stack_destroy(stack_t *self);

/**
 * Stack destructor (external allocation)
 *
 * @param [in/out] self  Stack instance
 */
void stack_free(stack_t *self);

/**
 * Push `value` to the stack
 *
 * @param [in/out] self   Stack instance
 * @param [in]     value  The value to push
 *
 * @return true on error, false otherwise
 */
bool stack_push(stack_t *self, stack_elem_t value);

/**
 * Retrieve TOS into `value`
 *
 * @param [in/out] self   Stack instance
 * @param [out]    value  The destination for TOS
 *
 * @return true on error, false otherwise
 */
bool stack_peek(stack_t *self, stack_elem_t *value);

/**
 * Pop `value` from the stack
 *
 * @param [in/out] self   Stack instance
 * @param [out]    value  NULL or destination pointer
 *
 * @return true on error, false otherwise
 */
bool stack_pop(stack_t *self, stack_elem_t *value);

/**
 * Resize the stack
 *
 * @param [in/out] self      Stack instance
 * @param [in]     capacity  New desired capacity (>= size)
 *
 * @return true on error, false otherwise
 */
bool stack_resize(stack_t *self, size_t capacity);

/**
 * Clear the stack
 *
 * @param [in/out] self  Stack instance
 */
void stack_clear(stack_t *self);

/**
 * Is stack empty?
 *
 * @param [in]  self  Stack instance
 *
 * @return true if self is empty, false otherwise
 */
bool stack_isEmpty(const stack_t *self);

#if STACK_USE_HASH_STRUCT
/**
 * Compute the stack struct's checksum
 *
 * @param [in]  self  Stack instance
 *
 * @return Checksum
 */
crc32_t stack_hashStruct(const stack_t *self);
#endif

#if STACK_USE_HASH_DATA
/**
 * Compute the stack data's checksum
 *
 * @param [in]  self  Stack instance
 *
 * @return Checksum
 */
crc32_t stack_hashData(const stack_t *self);
#endif

/**
 * Dump the stack (for debug)
 *
 * @param [in]  self  Stack instance
 */
void stack_dump(const stack_t *self);

/**
 * Validate the stack
 *
 * @param [in]  self  Stack instance
 *
 * @return Stack's validity
 */
stack_validity_e stack_validate(const stack_t *self);

/**
 * Retrieve a readable description for stack_validity
 *
 * @param [in]  self  Stack validity value
 *
 * @return Readable description
 */
const char *stack_validity_describe(stack_validity_e self);

/**
 * Retrieve a readable description for stack_allocState
 *
 * @param [in]  self  Stack allocState value
 *
 * @return Readable description
 */
const char *stack_allocState_describe(stack_allocState_e self);

#if STACK_USE_POISON
/**
 * Chack if `item` is poisoned
 *
 * @param [in]  item  The item to be checked
 *
 * @return true if `item` is poison, false otherwise
 */
bool stack_isPoison(const stack_elem_t *item);
#endif

/**
 * Check if ptr is a valid pointer
 *
 * @param [in]  ptr  The pointer to be checked
 *
 * @return true if `ptr` is valid, false otherwise
 */
bool isPointerValid(const void *ptr);

#if STACK_USE_CANARY
/**
 * Address of stack data's left canary
 *
 * @param [in]  self  Stack instance
 *
 * @return Left canary address
 */
static canary_t *stack_leftDataCanary(const stack_t *self);

/**
 * Address of stack data's right canary
 *
 * @param [in]  self  Stack instance
 *
 * @return Right canary address
 */
static canary_t *stack_rightDataCanary(const stack_t *self);
#endif

#ifdef TEST
void test_stack(stack_elem_t val1, stack_elem_t val2, stack_elem_t val3);
#endif

//--------------------------------------------------------------------------------

stack_t *stack_new(size_t capacity) {
    stack_t *self = (stack_t *)calloc(1, sizeof(stack_t));

    if (!isPointerValid(self)) {
        return NULL;
    }

    if (stack_construct(self, capacity) == NULL) {
        return NULL;
    }

    self->state = SAS_HEAP;

    #if STACK_USE_HASH_STRUCT
    self->structChecksum = stack_hashStruct(self);
    #endif

    #if STACK_USE_HASH_DATA
    self->dataChecksum = stack_hashData(self);
    #endif

    ASSERT_OK();

    return self;
}

stack_t *stack_construct(stack_t *self, size_t capacity) {
    if (!isPointerValid(self) || capacity == 0 || capacity >= STACK_HARD_CAP) {
        return NULL;
    }

    #if STACK_USE_CANARY
    self->leftCanary = CANARY;
    self->rightCanary = CANARY;
    #endif

    #if STACK_USE_HASH_STRUCT
    self->structChecksum = 0;
    #endif

    #if STACK_USE_HASH_DATA
    self->dataChecksum = 0;
    #endif

    self->capacity = capacity;
    self->size = 0;
    self->state = SAS_EXTERNAL;

    #if STACK_USE_CANARY
    self->data = (stack_elem_t *)calloc(1, capacity * sizeof(stack_elem_t) + sizeof(canary_t) * 2);
    #else
    self->data = (stack_elem_t *)calloc(capacity, sizeof(stack_elem_t));
    #endif

    if (!isPointerValid(self->data)) {
        return NULL;
    }

    #if STACK_USE_CANARY
    self->data = (stack_elem_t *)((canary_t *)self->data + 1);
    #endif

    #if STACK_USE_POISON
    memset(self->data, POISON, capacity * sizeof(stack_elem_t));
    #endif

    #if STACK_USE_CANARY
    *stack_leftDataCanary(self) = CANARY;
    *stack_rightDataCanary(self) = CANARY;
    #endif

    #if STACK_USE_HASH_STRUCT
    self->structChecksum = stack_hashStruct(self);
    #endif

    #if STACK_USE_HASH_DATA
    self->dataChecksum = stack_hashData(self);
    #endif

    ASSERT_OK();

    return self;
}

void stack_destroy(stack_t *self) {
    ASSERT_OK();

    REQUIRE(self->state == SAS_HEAP);

    self->state = SAS_EXTERNAL;

    #if STACK_USE_HASH_STRUCT
    self->structChecksum = stack_hashStruct(self);
    #endif

    #if STACK_USE_HASH_DATA
    self->dataChecksum = stack_hashData(self);
    #endif

    stack_free(self);

    free(self);
}

void stack_free(stack_t *self) {
    ASSERT_OK();

    REQUIRE(self->state == SAS_EXTERNAL);

    #if STACK_USE_CANARY
    free(stack_leftDataCanary(self));
    #else
    free(self->data);
    #endif

    #if STACK_USE_CANARY
    self->leftCanary = 0;
    self->rightCanary = 0;
    #endif

    #if STACK_USE_HASH_STRUCT
    self->structChecksum = 1;  // 0 would be the correct checksum for a null stack
    #endif

    #if STACK_USE_HASH_DATA
    self->dataChecksum = 1;  // Same as above
    #endif

    self->data = NULL;
    self->size = 0;
    self->capacity = 0;
    self->state = SAS_FREED;
}

bool stack_push(stack_t *self, stack_elem_t value) {
    ASSERT_OK();

    if (self->size + 1 > self->capacity) {
        // REQUIRE(self->capacity * 2 > self->capacity);  // Not necessary anymore, because STACK_HARD_CAP ensures no overflows

        if (stack_resize(self, self->capacity * 2)) {
            ASSERT_OK();

            return true;
        }
    }

    self->data[self->size++] = value;

    #if STACK_USE_HASH_STRUCT
    self->structChecksum = stack_hashStruct(self);
    #endif

    #if STACK_USE_HASH_DATA
    self->dataChecksum = stack_hashData(self);
    #endif

    ASSERT_OK();

    return false;
}

bool stack_peek(stack_t *self, stack_elem_t *value) {
    ASSERT_OK();

    if (stack_isEmpty(self)) {
        return true;
    }

    *value = self->data[self->size - 1];

    ASSERT_OK();

    return false;
}

bool stack_pop(stack_t *self, stack_elem_t *value) {
    ASSERT_OK();

    if (stack_isEmpty(self)) {
        return true;
    }

    self->size--;
    if (value != NULL) {
        *value = self->data[self->size];
    }

    #if STACK_USE_POISON
    memset(self->data + self->size, POISON, sizeof(stack_elem_t));
    #endif

    #if STACK_USE_HASH_STRUCT
    self->structChecksum = stack_hashStruct(self);
    #endif

    #if STACK_USE_HASH_DATA
    self->dataChecksum = stack_hashData(self);
    #endif

    if (self->size * 4 + 4 < self->capacity) {
        if (stack_resize(self, self->capacity / 2)) {
            ASSERT_OK();

            return true;
        }
    }

    ASSERT_OK();

    return false;
}

bool stack_resize(stack_t *self, size_t capacity) {
    ASSERT_OK();

    if (capacity <= self->size || capacity >= STACK_HARD_CAP) {
        return true;
    }

    // After some thinking, I decided to abandon stack unpoisoning,
    // because right after the newly reallocated memory will be a malloc-header,
    // which is likely to be unequal to poison anyway
    /*
    #if STACK_USE_POISON
    if (capacity < self->capacity) {
        memset(self->data + capacity, 0x00, (self->capacity - capacity) * sizeof(stack_elem_t));
    }
    #endif // STACK_USE_POISON
    */

    #if STACK_USE_CANARY
    stack_elem_t *newData = (stack_elem_t *)realloc(stack_leftDataCanary(self), capacity * sizeof(stack_elem_t) + 2 * sizeof(canary_t));
    #else
    stack_elem_t *newData = (stack_elem_t *)realloc(self->data, capacity * sizeof(stack_elem_t));
    #endif
    if (!isPointerValid(newData)) {
        return true;
    }

    #if STACK_USE_CANARY
    self->data = (stack_elem_t *)((canary_t *)newData + 1);
    #else
    self->data = newData;
    #endif

    #if STACK_USE_POISON
    if (capacity > self->capacity) {
        memset(self->data + self->capacity, POISON, (capacity - self->capacity) * sizeof(stack_elem_t));
    }
    #endif

    self->capacity = capacity;

    #if STACK_USE_CANARY
    *stack_leftDataCanary(self) = CANARY;
    *stack_rightDataCanary(self) = CANARY;
    #endif

    #if STACK_USE_HASH_STRUCT
    self->structChecksum = stack_hashStruct(self);
    #endif

    #if STACK_USE_HASH_DATA
    self->dataChecksum = stack_hashData(self);
    #endif

    ASSERT_OK();

    return false;
}

void stack_clear(stack_t *self) {
    ASSERT_OK();

    while (!stack_isEmpty(self)) {
        stack_pop(self, NULL);
    }

    ASSERT_OK();
}

bool stack_isEmpty(const stack_t *self) {
    ASSERT_OK();

    return self->size == 0;
}

#if STACK_USE_HASH_STRUCT
crc32_t stack_hashStruct(const stack_t *self) {
    //ASSERT_OK(); // Inapplicable!
    REQUIRE(isPointerValid(self));

    crc32_t checksum = 0;

    #define HASH_FIELD_(field) \
        checksum = crc32_update(checksum, (const char *)&self->field, sizeof(self->field));

    HASH_FIELD_(size);
    HASH_FIELD_(capacity);
    HASH_FIELD_(state);
    HASH_FIELD_(data);  // Hashes the pointer value to avoid relocation.

    #undef HASH_FIELD_

    //ASSERT_OK(); // Same as above!; Also not really necessary)

    return checksum;
}
#endif

#if STACK_USE_HASH_DATA
crc32_t stack_hashData(const stack_t *self) {
    //ASSERT_OK(); // Inapplicable!
    REQUIRE(isPointerValid(self));
    REQUIRE(isPointerValid(self->data));
    REQUIRE(self->capacity > 0);

    crc32_t checksum = crc32_compute((const char *)self->data, self->capacity * sizeof(stack_elem_t));

    //ASSERT_OK(); // Same as above!; Also not really necessary)

    return checksum;
}
#endif

void stack_dump(const stack_t *self) {
    #ifdef STACK_ELEM_PRINT

    printf("[WARNING: STACK_ELEM_PRINT is specified, so the dump may fail through the user\'s fault.]\n");

    #endif

    stack_validity_e validity = stack_validate(self);

    printf("stack_t (%s) [0x%p] {\n", stack_validity_describe(validity), self);
    if (isPointerValid(self)) {
        #if STACK_USE_CANARY
        printf("  left canary     = 0x%016llX\n", self->leftCanary);
        #endif

        printf("  size            = %zu\n", self->size);
        printf("  capacity        = %zu (out of %zu)\n", self->capacity, STACK_HARD_CAP);
        printf("  state           = %s\n", stack_allocState_describe(self->state));

        #if STACK_USE_HASH_STRUCT
        printf("  struct checksum = 0x%08X\n", self->structChecksum);
        #endif

        #if STACK_USE_HASH_DATA
        printf("  data checksum   = 0x%08X\n", self->dataChecksum);
        #endif

        #if STACK_USE_CANARY
        printf("  right canary    = 0x%016llX\n", self->rightCanary);
        #endif

        printf("  data [0x%p] {\n", self->data);
        if (isPointerValid(self->data)) {
            #if STACK_USE_CANARY
            printf("    l. canary     = 0x%016llX\n", *stack_leftDataCanary(self));
            #endif

            size_t limit = self->capacity;
            if (STACK_DUMP_LIMIT < limit) {
                limit = STACK_DUMP_LIMIT;
            }

            for (size_t i = 0; i < limit; ++i) {
                char lineMarker = 0;

                if (i < self->size) {
                    lineMarker = '*';
                } else
                #if STACK_USE_POISON
                if (stack_isPoison(self->data + i)) {
                    lineMarker = '~';
                } else
                #endif
                {
                    lineMarker = ' ';
                }

                printf("  %c [%2zu] = ", lineMarker, i);

                printf("0x");
                for (size_t j = 0; j < sizeof(stack_elem_t); ++j) {
                    printf("%02X", ((unsigned char *)(self->data + i))[j]);
                }
                printf(" ");

                #ifdef STACK_ELEM_PRINT

                // Okay, we'll assume we may trust this 'function'...
                printf("(");
                STACK_ELEM_PRINT(self->data[i]);
                printf(")");

                #endif

                printf("\n");
            }

            if (limit < self->capacity) {
                printf("    ...\n");
            } else {
                #if STACK_USE_CANARY
                printf("    r. canary     = 0x%016llX\n", *stack_rightDataCanary(self));
                #endif
            }
        } else {
            printf("    <corrupt>\n");
        }

        printf("  }\n");
    } else {
        printf("  <corrupt>\n");
    }

    printf("}\n\n");

    fflush(stdout);
    fflush(stderr);
}

stack_validity_e stack_validate(const stack_t *self) {
    if (!isPointerValid(self) || !isPointerValid(self->data)) {
        return STACK_BADPTR;
    }

    if (self->capacity == 0 || self->size > self->capacity || \
        self->capacity >= STACK_HARD_CAP || !isPointerValid(self->data + self->capacity)) {
        return STACK_BADSIZE;
    }

    if (self->state == SAS_FREED) {
        return STACK_UAF;
    }

    #if STACK_USE_CANARY
    if (self->leftCanary != CANARY || self->rightCanary != CANARY) {
        return STACK_BADCANARY;
    }
    #endif

    // Order is important, because the first hash validates capacity, and the second relies on it
    #if STACK_USE_HASH_STRUCT
    if (self->structChecksum != stack_hashStruct(self)) {
        return STACK_BADHASH;
    }
    #endif

    // -"-
    #if STACK_USE_HASH_DATA
    if (self->dataChecksum   != stack_hashData(self)) {
        return STACK_BADHASH;
    }
    #endif

    #if STACK_USE_POISON
    if (self->size + 1 <= self->capacity && !stack_isPoison(self->data + self->size)) {
        return STACK_BADPOISON;
    }
    #endif

    #if STACK_USE_CANARY
    if ((isPointerValid(stack_leftDataCanary(self)) && *stack_leftDataCanary(self) != CANARY) || \
        (isPointerValid(stack_rightDataCanary(self)) && *stack_rightDataCanary(self) != CANARY)) {
        return STACK_BADCANARY;
    }
    #endif

    // TODO

    return STACK_VALID;
}

#define DESCRIBE_(value, descr)  case value: return "<" #value "> " descr;
const char *stack_validity_describe(stack_validity_e self) {
    switch (self) {
        case STACK_VALID: return "ok";

        DESCRIBE_(STACK_BADPTR,    "Bad pointer")
        DESCRIBE_(STACK_BADSIZE,   "Bad size")
        DESCRIBE_(STACK_BADCANARY, "Bad canary")
        DESCRIBE_(STACK_BADHASH,   "Bad hash")
        DESCRIBE_(STACK_BADPOISON, "Poison abscent")
        DESCRIBE_(STACK_UAF,       "Use after free")


        default: return "!CORRUPT VALIDITY!";
    }
}

const char *stack_allocState_describe(stack_allocState_e self) {
    switch (self) {
        DESCRIBE_(SAS_EXTERNAL, "external")
        DESCRIBE_(SAS_HEAP,     "heap")
        DESCRIBE_(SAS_FREED,    "freed")

        default: return "!CORRUPT ALLOCSTATE!";
    }
}
#undef DESCRIBE_

#if STACK_USE_POISON
bool stack_isPoison(const stack_elem_t *item) {
    const unsigned char *charItem = (const unsigned char *)item;
    for (size_t i = 0; i < sizeof(stack_elem_t); ++i) {
        if (charItem[i] != POISON) {
            return 0;
        }
    }
    return 1;
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

    #ifdef _WIN32

    // (c) Ded32, TXLib
    MEMORY_BASIC_INFORMATION mbi = {};
    if (!VirtualQuery(ptr, &mbi, sizeof (mbi)))
        return false;

    if (mbi.Protect & (PAGE_GUARD | PAGE_NOACCESS))
        return false;  // Guard page -> bad ptr

    DWORD readRights = PAGE_READONLY | PAGE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY;

    return (mbi.Protect & readRights) != 0;

    #endif // _WIN32
}



#if STACK_USE_CANARY
static canary_t *stack_leftDataCanary(const stack_t *self) {
    REQUIRE(isPointerValid(self));
    REQUIRE(isPointerValid(self->data));

    return (canary_t *)self->data - 1;

}

static canary_t *stack_rightDataCanary(const stack_t *self) {
    REQUIRE(isPointerValid(self));
    REQUIRE(isPointerValid(self->data));

    return (canary_t *)(self->data + self->capacity);
}
#endif

//================================================================================

#ifdef TEST
void test_stack(stack_elem_t val1, stack_elem_t val2, stack_elem_t val3) {
    stack_t stk = {};
    TEST_ASSERT(stack_construct(&stk, 10));

    stack_elem_t tmp = val3;

    TEST_ASSERT(stack_isEmpty(&stk));

    TEST_ASSERT(stack_push(&stk, val1) == 0);
    TEST_ASSERT(stack_push(&stk, val2) == 0);

    TEST_ASSERT(!stack_isEmpty(&stk));
    TEST_ASSERT(stk.size == 2);


    TEST_ASSERT(stack_peek(&stk, &tmp) == 0 && tmp == val2);
    TEST_ASSERT_M(stack_peek(&stk, &tmp) == 0 && tmp == val2, "Peek mustn\'t alter the stack");

    TEST_ASSERT(stack_pop(&stk, &tmp) == 0 && tmp == val2);
    TEST_ASSERT(stack_pop(&stk, &tmp) == 0 && tmp == val1);

    TEST_ASSERT(stack_isEmpty(&stk));


    TEST_ASSERT(stack_push(&stk, val1) == 0);
    TEST_ASSERT(stack_push(&stk, val2) == 0);

    TEST_ASSERT(stack_resize(&stk, 20) == 0);
    TEST_ASSERT(!stack_isEmpty(&stk));
    TEST_ASSERT(stk.size == 2);
    TEST_ASSERT(stack_peek(&stk, &tmp) == 0 && tmp == val2);

    stack_clear(&stk);
    TEST_ASSERT(stack_isEmpty(&stk));

    stack_free(&stk);


    stack_t *stk2 = stack_new(123);

    TEST_ASSERT(stack_resize(stk2, 8) == 0);
    TEST_ASSERT(stk2->capacity == 8);

    stack_destroy(stk2);
}
#endif // TEST

#undef ASSERT_OK
#undef REQUIRE

#undef STACK_USE_HASH_STRUCT
#undef STACK_USE_HASH_DATA
#undef STACK_USE_POISON
#undef STACK_USE_CANARY

#undef STACK_VALIDATION_LEVEL
#undef STACK_ELEM_PRINT

#endif // STACK_H_GUARD

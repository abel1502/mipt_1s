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
#include <stdint.h>

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
enum bool { false, true };
#endif

#if STACK_USE_POISON
static const unsigned char POISON = 0xAA;
#endif

#if STACK_USE_CANARY
typedef unsigned long long canary_t;
//static const canary_t CANARY = ((canary_t)-1 << 32) | 0xDEADB1AD;  // DEADBIRD
static const canary_t CANARY = ((canary_t)-1) & 0xBADC0DEDEADB1AD;  // BAD CODE DEAD BIRD
#endif

/**
 * A type that holds a CRC32 checksum
 */
typedef uint32_t crc32_t;

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

/**
 * Compute a CRC32 of `size` bytes of `data`
 *
 * @param [in]  data  The staring offset to be hashed
 * @param [in]  size  The byte size of data to be hashed
 *
 * @return The resulting hash
 */
crc32_t crc32_compute(const char *data, size_t size);

/**
 * Update `initial` CRC32 with the hash of `size` bytes of `data`
 *
 * @param [in]  initial  The initial hash value
 * @param [in]  data     The staring offset to be hashed
 * @param [in]  size     The byte size of data to be hashed
 *
 * @return The resulting hash
 */
crc32_t crc32_update(crc32_t initial, const char *data, size_t size);

#ifdef TEST
void test_crc32();

void test_stack(stack_elem_t val1, stack_elem_t val2, stack_elem_t val3);
#endif // TEST

//--------------------------------------------------------------------------------

static const crc32_t CRC32Table[] = { /* CRC polynomial 0xedb88320; Generated by (C) 1986 Gary S. Brown. */
0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

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

crc32_t crc32_update(crc32_t value, const char *data, size_t size) {
    REQUIRE(isPointerValid(data));

    value ^= 0xFFFFFFFF;

    for (size_t i = 0; i < size; ++i) {
        value = (value << 8) ^ CRC32Table[(value ^ data[i]) & 0xFF];
    }

    value ^= 0xFFFFFFFF;

    return value;
}

crc32_t crc32_compute(const char *data, size_t size) {
    REQUIRE(isPointerValid(data));

    return crc32_update(0, data, size);
}

#ifdef TEST
void test_crc32() {
    TEST_ASSERT_M(sizeof(crc32_t) == 4, "CRC32 is 32-bit");

    char data1[] = "Hello, World!";
    size_t size1 = strlen(data1);
    TEST_ASSERT_M(crc32_compute(data1, size1) == crc32_compute(data1, size1), "CRC32 is the same for the same input");

    char data2[] = "And something else.";
    size_t size2 = strlen(data2);
    TEST_ASSERT_M(crc32_compute(data1, size1) != crc32_compute(data2, size2), "CRC32 is different for different inputs");

    crc32_t checksum_together = crc32_compute("ABCDEFGH", 8);
    crc32_t checksum_partial = crc32_update(0, "ABCD", 4);
    checksum_partial = crc32_update(checksum_partial, "EFGH", 4);
    TEST_ASSERT_M(checksum_together == checksum_partial, "CRC32 is stream-compatible");

    TEST_ASSERT_M(crc32_compute("AB", 2) != crc32_compute("BA", 2), "CRC32 is order-dependent");
}

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

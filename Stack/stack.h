/**
 * @file stack.h
 * A secure stack library
 *
 * To use, you should/may define the following:
 *
 * - [typedef] (required) stack_elem_t @n
 * The type of elements on which the stack works
 *
 * - [define]  (optional) STACK_ELEM_PRINT @n
 * Name of the helper function to print stack elements during debug dump.
 * Signature: void STACK_ELEM_PRINT(stack_elem_t). If not defined, the dump prints raw bytes in hex.
 *
 * - [define]  (optional) STACK_VALIDATION_LEVEL @n
 * A number from 0 to <TODO>, specifying, how much validation will be performed. Defaults to maximum.
 * (@emoji :warning: Actually doesn't work yet)
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
 - Protect the stack as hell)
 # Define ASSERT_OK
 ? Remove asserts from destructors
 ? Change peek and pop to output pointer instead of rval?
 - Add error codes
 # Rework destructor SAS checks
 ? Macro-based overloads
 - Add release-time checks (+in constructors)
 - Make heavy debug checks conditionally compile
 # Make ASSERT_OK a functional-style marco
 # Poison
 # Canaries
 - Data Canaries
 # Hashes
 # isPointerValid
 # CRC32 lib
 # Unit tests
 ? Rework stack_clear not to rely on stack_pop
 ...
    =========================
*/


#ifndef STACK_H_GUARD
#define STACK_H_GUARD

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "checksum.h"

//--------------------------------------------------------------------------------

static const unsigned char POISON = 0xAA;

typedef unsigned long long canary_t;
static const canary_t CANARY = ((canary_t)-1 << 32) | 0xDEADB1AD;  // DEADBIRD

typedef struct stack_s stack_t;

typedef enum {
    SAS_USERSPACE,
    SAS_HEAP,
    SAS_FREED
} stack_allocState_e;

struct stack_s {
    canary_t leftCanary;
    stack_elem_t *data;
    size_t size;
    size_t capacity;
    crc32_t structChecksum;
    crc32_t dataChecksum;
    stack_allocState_e state;
    canary_t rightCanary;
};

typedef enum {
    STACK_VALID,
    STACK_BADPTR,
    STACK_BADSIZE,
    STACK_BADCANARY,
    STACK_BADHASH,
    STACK_BADPOISON,
    STACK_UAF
} stack_validity_e;


stack_t *stack_new(size_t capacity);

stack_t *stack_construct(stack_t *self, size_t capacity);

void stack_destroy(stack_t *self);

void stack_free(stack_t *self);

void stack_push(stack_t *self, stack_elem_t value);

stack_elem_t stack_peek(stack_t *self);

stack_elem_t stack_pop(stack_t *self);

void stack_resize(stack_t *self, size_t capacity);

void stack_clear(stack_t *self);

int stack_isEmpty(stack_t *self);

crc32_t stack_hashStruct(stack_t *self);

crc32_t stack_hashData(stack_t *self);

void stack_dump(stack_t *self);

stack_validity_e stack_validate(stack_t *self);

const char *stack_validity_describe(stack_validity_e self);

const char *stack_allocState_describe(stack_allocState_e self);

int stack_isPoison(stack_elem_t *item);

int isPointerValid(void *ptr);

#ifdef TEST
void test_stack(stack_elem_t val1, stack_elem_t val2, stack_elem_t val3);
#endif // TEST

//--------------------------------------------------------------------------------

#ifndef MACROFUNC  // Same as in tests, but may not be included
#define MACROFUNC(...) do {__VA_ARGS__} while (0)
#endif

#define ASSERT_OK()  MACROFUNC(                                                          \
    if (stack_validate(self) != STACK_VALID) {                                           \
        fprintf(stderr, "==============[ !!! CRITICAL FAILURE !!! ]==============\n");   \
        fprintf(stderr, "              (or not, but suck it anyway)              \n\n"); \
        stack_dump(self);                                                                \
        fprintf(stderr, "========================================================\n");   \
        abort();                                                                         \
    } )


stack_t *stack_new(size_t capacity) {
    stack_t *self = (stack_t *)calloc(1, sizeof(stack_t));

    assert(self != NULL); // TODO

    stack_construct(self, capacity);

    self->state = SAS_HEAP;

    return self;
}

stack_t *stack_construct(stack_t *self, size_t capacity) {
    assert(isPointerValid(self));
    assert(capacity > 0);

    self->leftCanary = CANARY;
    self->capacity = capacity;
    self->size = 0;
    self->data = (stack_elem_t *)calloc(capacity, sizeof(stack_elem_t));
    self->structChecksum = 0;
    self->dataChecksum = 0;
    self->state = SAS_USERSPACE;
    self->rightCanary = CANARY;

    assert(self->data != NULL); // TODO

    memset(self->data, POISON, capacity * sizeof(stack_elem_t));

    self->structChecksum = stack_hashStruct(self);
    self->dataChecksum = stack_hashData(self);

    ASSERT_OK();


    return self;
}

void stack_destroy(stack_t *self) {
    ASSERT_OK();

    assert(self->state == SAS_HEAP);
    self->state = SAS_USERSPACE;

    stack_free(self);

    free(self);
}

void stack_free(stack_t *self) {
    ASSERT_OK();

    assert(self->state == SAS_USERSPACE);

    free(self->data);

    self->leftCanary = 0;
    self->data = NULL;
    self->size = 0;
    self->capacity = 0;
    self->state = SAS_FREED;
    self->structChecksum = 1;  // 0 would be the correct checksum for a null stack
    self->dataChecksum = 1;  // Same as above
    self->rightCanary = 0;
}

void stack_push(stack_t *self, stack_elem_t value) {
    ASSERT_OK();

    if (self->size + 1 > self->capacity) {
        stack_resize(self, self->capacity * 2);
    }

    self->data[self->size++] = value;

    self->structChecksum = stack_hashStruct(self);
    self->dataChecksum = stack_hashData(self);

    ASSERT_OK();
}

stack_elem_t stack_peek(stack_t *self) {
    ASSERT_OK();

    assert(!stack_isEmpty(self));

    return self->data[self->size - 1];
}

stack_elem_t stack_pop(stack_t *self) {
    ASSERT_OK();

    assert(!stack_isEmpty(self));

    stack_elem_t res = self->data[--(self->size)];

    memset(self->data + self->size, POISON, sizeof(stack_elem_t));

    self->structChecksum = stack_hashStruct(self);
    self->dataChecksum = stack_hashData(self);

    ASSERT_OK();

    return res;
}

void stack_resize(stack_t *self, size_t capacity) {
    ASSERT_OK();

    assert(capacity > self->size);

    stack_elem_t *newData = (stack_elem_t *)realloc(self->data, capacity * sizeof(stack_elem_t));
    assert(newData != NULL);

    self->data = newData;

    if (capacity > self->capacity) {
        memset(self->data + self->capacity, POISON, (capacity - self->capacity) * sizeof(stack_elem_t));
    }

    self->capacity = capacity;

    self->structChecksum = stack_hashStruct(self);
    self->dataChecksum = stack_hashData(self);

    ASSERT_OK();
}

void stack_clear(stack_t *self) {
    ASSERT_OK();

    while (!stack_isEmpty(self)) {
        stack_pop(self);
    }

    ASSERT_OK();
}

int stack_isEmpty(stack_t *self) {
    ASSERT_OK();

    return self->size == 0;
}

crc32_t stack_hashStruct(stack_t *self) {
    //ASSERT_OK(); // Inapplicable!
    assert(self != NULL);

    crc32_t checksum = 0;

    #define HASH_FIELD(field) \
        checksum = crc32_update(checksum, (const char *)&self->field, sizeof(self->field));

    HASH_FIELD(size);
    HASH_FIELD(capacity);
    HASH_FIELD(state);
    HASH_FIELD(data);  // Hashes the pointer value to avoid relocation. (Don't know why, though).

    #undef HASH_FIELD

    //ASSERT_OK(); // Same as above!; Also not really necessary)

    return checksum;
}

crc32_t stack_hashData(stack_t *self) {
    //ASSERT_OK(); // Inapplicable!
    assert(self != NULL);
    assert(self->data != NULL);
    assert(self->capacity > 0);

    crc32_t checksum = crc32_compute((const char *)self->data, self->capacity * sizeof(stack_elem_t));

    //ASSERT_OK(); // Same as above!; Also not really necessary)

    return checksum;
}

void stack_dump(stack_t *self) {
    #if defined(STACK_ELEM_PRINT)

    printf("[WARNING: STACK_ELEM_PRINT is specified, so the dump may fail through the user\'s fault.]\n");

    #endif

    stack_validity_e validity = stack_validate(self);

    printf("stack_t (%s) [0x%p] {\n", stack_validity_describe(validity), self);
    if (isPointerValid(self)) {
        printf("  left canary     = 0x%016llX\n", self->leftCanary);
        printf("  size            = %zu\n", self->size);
        printf("  capacity        = %zu\n", self->capacity);
        printf("  state           = %s\n", stack_allocState_describe(self->state));
        printf("  struct checksum = 0x%08X\n", self->structChecksum);
        printf("  data checksum   = 0x%08X\n", self->dataChecksum);
        printf("  right canary    = 0x%016llX\n", self->rightCanary);

        printf("  data [0x%p] {\n", self->data);
        if (isPointerValid(self->data)) {
            size_t limit = self->capacity;
            if (100 < limit) {
                limit = 100;
            }

            for (size_t i = 0; i < limit; ++i) {
                printf("  %c [%2zu] = ", \
                       i < self->size ? '*' : stack_isPoison(self->data + i) ? '~' : ' ', \
                       i);

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

            if (limit < self->size) {
                printf("    ...\n");
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

stack_validity_e stack_validate(stack_t *self) {
    if (!isPointerValid(self) || !isPointerValid(self->data)) {
        return STACK_BADPTR;
    }

    if (self->capacity == 0 || self->size > self->capacity) {
        return STACK_BADSIZE;
    }

    if (self->state == SAS_FREED) {
        return STACK_UAF;
    }

    if (self->leftCanary != CANARY || self->rightCanary != CANARY) {
        return STACK_BADCANARY;
    }

    if (self->structChecksum != stack_hashStruct(self) || \
        self->dataChecksum   != stack_hashData(self)) {
        // Order is important, because the first hash validates capacity, and the second relies on it
        return STACK_BADHASH;
    }

    if (self->size + 1 <= self->capacity && !stack_isPoison(self->data + self->size)) {
        return STACK_BADPOISON;
    }

    // TODO

    return STACK_VALID;
}

const char *stack_validity_describe(stack_validity_e self) {
    switch (self) {
    case STACK_VALID:
        return "ok";
    case STACK_BADPTR:
        return "BAD POINTER";
    case STACK_BADSIZE:
        return "BAD SIZE";
    case STACK_BADCANARY:
        return "BAD CANARY";
    case STACK_BADHASH:
        return "BAD CHECKSUM";
    case STACK_BADPOISON:
        return "POISON ABSCENT";
    case STACK_UAF:
        return "USE AFTER FREE";
    default:
        return "!CORRUPT VALIDITY!";
    }
}

const char *stack_allocState_describe(stack_allocState_e self) {
    switch (self) {
    case SAS_USERSPACE:
        return "userspace";
    case SAS_HEAP:
        return "heap";
    case SAS_FREED:
        return "freed";
    default:
        return "!CORRUPT ALLOCSTATE!";
    }
}

int stack_isPoison(stack_elem_t *item) {
    unsigned char *charItem = (unsigned char *)item;
    for (size_t i = 0; i < sizeof(stack_elem_t); ++i) {
        if (charItem[i] != POISON) {
            return 0;
        }
    }
    return 1;
}

int isPointerValid(void *ptr) {
    return ptr >= (void *)4096 \
        && ptr <= (void *)(~((size_t)1) >> 2);
        // I know this feels crotchy, but it essentially says that
        // the lowest and the highest addresses are definitely bad
}

#ifdef TEST
void test_stack(stack_elem_t val1, stack_elem_t val2, stack_elem_t val3) {
    stack_t stk = {};
    stack_construct(&stk, 10);

    TEST_ASSERT(stack_isEmpty(&stk));

    stack_push(&stk, val1);
    stack_push(&stk, val2);

    TEST_ASSERT(!stack_isEmpty(&stk));
    TEST_ASSERT(stk.size == 2);

    TEST_ASSERT(stack_peek(&stk) == val2);
    TEST_ASSERT_M(stack_peek(&stk) == val2, "Peek mustn\'t alter the stack");

    TEST_ASSERT(stack_pop(&stk) == val2);
    TEST_ASSERT(stack_pop(&stk) == val1);

    TEST_ASSERT(stack_isEmpty(&stk));


    stack_push(&stk, val3);
    stack_push(&stk, val2);

    stack_resize(&stk, 20);
    TEST_ASSERT(!stack_isEmpty(&stk));
    TEST_ASSERT(stk.size == 2);
    TEST_ASSERT(stack_peek(&stk) == val2);

    stack_clear(&stk);
    TEST_ASSERT(stack_isEmpty(&stk));

    stack_free(&stk);
}
#endif // TEST

#undef ASSERT_OK

#endif // STACK_H_GUARD

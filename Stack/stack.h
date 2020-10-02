/**
 * A secure stack library
 *
 * To use, type-define stack_elem_t type and include this file.
 *
 * You may also #define a STACK_ELEM_PRINT function with the signature:
 * (stack_elem_t) -> void
 * It shall be used during the debug dump
 *
 * Example:
 *   typedef double stack_elem_t;
 *   #include "stack.h"
 *
 */


/*
    =====[ Global TODO ]=====
 # Write a basic stack
 - Protect the stack as hell)
 # Define ASSERT_OK
 ? Remove asserts from destructors
 ? Change peek and pop to output pointer instead of rval?
 ? Add error codes
 # Rework destructor SAS checks
 - Macro-based overloads
 - Add release-time checks (+in constructors)
 - Make heavy debug checks conditionally compile
 - Make ASSERT_OK a functional-style marco
 - Poison
 - Canaries
 - Hashes
 ...
    =========================
*/


#ifndef STACK_H_GUARD
#define STACK_H_GUARD

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

//--------------------------------------------------------------------------------

typedef struct stack_s stack_t;

typedef enum {
    SAS_USERSPACE,
    SAS_HEAP,
    SAS_FREED
} stack_allocState_e;

struct stack_s {
    stack_elem_t *data;
    size_t size;
    size_t capacity;
    stack_allocState_e state;
};

typedef enum {
    STACK_VALID,
    STACK_BADPTR,
    STACK_BADSIZE,
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

void stack_dump(stack_t *self);

stack_validity_e stack_validate(stack_t *self);

const char *stack_describeValidity(stack_validity_e validity);

//--------------------------------------------------------------------------------

#define MACROFUNC(...) do {__VA_ARGS__} while (0)

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

    stack_construct(self, capacity);

    self->state = SAS_HEAP;

    return self;
}

stack_t *stack_construct(stack_t *self, size_t capacity) {
    assert(self != NULL);
    assert(capacity > 0);

    self->capacity = capacity;
    self->size = 0;
    self->data = (stack_elem_t *)calloc(capacity, sizeof(stack_elem_t));
    self->state = SAS_USERSPACE;

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

    self->size = 0;
    self->capacity = 0;
    self->state = SAS_FREED;
}

void stack_push(stack_t *self, stack_elem_t value) {
    ASSERT_OK();

    if (self->size + 1 > self->capacity) {
        stack_resize(self, self->capacity * 2);
    }

    self->data[self->size++] = value;

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

    return self->data[--(self->size)];
}

void stack_resize(stack_t *self, size_t capacity) {
    ASSERT_OK();

    assert(capacity > self->size);

    stack_elem_t *newData = (stack_elem_t *)realloc(self->data, capacity * sizeof(stack_elem_t));
    assert(newData != NULL);

    self->data = newData;

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

void stack_dump(stack_t *self) {
    stack_validity_e validity = stack_validate(self);

    printf("stack_t (%s) [0x%p] {\n", stack_describeValidity(validity), self);
    if (self != NULL) {
        printf("  size     = %zu\n", self->size);
        printf("  capacity = %zu\n", self->capacity);
        printf("  state    = %s\n", (self->state == SAS_USERSPACE) ? \
                                    "userspace" : \
                                    (self->state == SAS_HEAP) ? \
                                    "heap" : \
                                    (self->state == SAS_FREED) ? \
                                    "freed" : \
                                    "CORRUPT");
        printf("  data [0x%p] {\n", self->data);
        if (self->data != NULL) {
            size_t limit = self->size;
            if (self->capacity < limit) {
                limit = self->capacity;
            }
            if (100 < limit) {
                limit = 100;
            }

            for (size_t i = 0; i < self->size; ++i) {
                printf("    [%2zu] = 0x", i);

                // We do this so weirdly because the type is unknown and
                // trusting the user to provide a format specifier is unsafe
                for (size_t j = 0; j < sizeof(stack_elem_t); ++j) {
                    printf("%02X", ((unsigned char *)(self->data + i))[j]);
                }

                printf("\n");
            }

            if (limit < self->size) {
                printf("    ...");
            }
        } else {
            printf("    <corrupt>\n");
        }

        printf("  }\n");
    } else {
        printf("  <corrupt>\n");
    }

    printf("}\n\n");
}

stack_validity_e stack_validate(stack_t *self) {
    if (self == NULL || self->data == NULL) {
        return STACK_BADPTR;
    }

    if (self->capacity == 0 || self->size > self->capacity) {
        return STACK_BADSIZE;
    }

    if (self->state == SAS_FREED) {
        return STACK_UAF;
    }

    // TODO

    return STACK_VALID;
}

const char *stack_describeValidity(stack_validity_e validity) {
    switch (validity) {
    case STACK_VALID:
        return "ok";
    case STACK_BADPTR:
        return "BAD POINTER";
    case STACK_BADSIZE:
        return "BAD SIZE";
    case STACK_UAF:
        return "USE AFTER FREE";
    default:
        return "!CORRUPT VALIDITY!";
    }
}

#undef ASSERT_OK

#endif // STACK_H_GUARD

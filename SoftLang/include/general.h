#ifndef GENERAL_H_GUARD
#define GENERAL_H_GUARD

#include <new>


#define ERR(msg, ...) err_(__func__, __LINE__, msg, ##__VA_ARGS__)


/*  Alright, with new requirements come new standards.
    Now ctor must return zero on success and non-zero on failure
    (Type doesn't matter for the factory methods)
    It's also preferable (though not required) that the default ctor
    always returns 0 (essentially a c-style noexcept) */
#define FACTORIES(CLS)                          \
    static CLS *create() {                      \
        CLS *self = new (std::nothrow) CLS();   \
                                                \
        if (!self)  return nullptr;             \
                                                \
        if (self->ctor()) {                     \
            self->destroy();                    \
            return nullptr;                     \
        }                                       \
                                                \
        return self;                            \
    }                                           \
                                                \
    void destroy() {                            \
        dtor();                                 \
        delete this;                            \
    }


// TODO: Maybe implement a traceback stack and error messages?


#define TMPVARNAME$_HELPER1(A, B)    A ## B
#define TMPVARNAME  TMPVARNAME$_HELPER1(tmp_, __LINE__)

// The TRY macro is intended for use to forward any error (i.e. non-zero) return code
// from an expression out of the context function. Useful for work with ctor's in the
// new format.
#define TRY(STMT)  TRY_C(STMT, )

// Try not (for when success is represented with a non-zero value - a pointer, for example)
#define TRY_N(STMT)  TRY_NC(STMT, )

// Boolean try
#define TRY_B(STMT)  TRY_BC(STMT, )

// Try with cleanup
#define TRY_C(STMT, CLEANUP)  {                         \
    auto TMPVARNAME = (STMT);                           \
    if (TMPVARNAME) {                                   \
        if (verbosity >= 3) {                           \
            ERR("Error caught in \"%s\"", #STMT);       \
        }                                               \
        CLEANUP;                                        \
        return TMPVARNAME;                              \
    }                                                   \
}

// Try not with cleanup
#define TRY_NC(STMT, CLEANUP)  {                         \
    auto TMPVARNAME = (STMT);                           \
    if (!TMPVARNAME) {                                  \
        if (verbosity >= 3) {                           \
            ERR("Error caught in \"%s\"", #STMT);       \
        }                                               \
        CLEANUP;                                        \
        return TMPVARNAME;                              \
    }                                                   \
}

// Boolean try with cleanup
#define TRY_BC(STMT, CLEANUP)  {    \
    if (STMT) {                     \
        CLEANUP;                    \
        return true;                \
    }                               \
}



// TODO: Throw
#define REQUIRE(STMT)                                   \
    if (!(STMT)) {                                      \
        ERR("Requirement not satisfied: %s\n", #STMT);  \
        abort();                                        \
    }


extern int verbosity;

void err_(const char *funcName, int lineNo, const char *msg, ...);


#endif // GENERAL_H_GUARD


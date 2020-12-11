#ifndef GENERAL_H_GUARD
#define GENERAL_H_GUARD

#include <new>

#define ERR(msg, ...) err_(__func__, __LINE__, msg, ##__VA_ARGS__)


#define FACTORIES(CLS)                          \
    static CLS *create() {                      \
        /* Auto-throws error */                 \
        CLS *self = new (std::nothrow) CLS();   \
                                                \
        if (!self)  return nullptr;             \
                                                \
        return self->ctor();                    \
    }                                           \
                                                \
    void destroy() {                            \
        dtor();                                 \
        delete this;                            \
    }


#pragma GCC diagnostic ignored "-Wshadow"  // If we pop after here, the warning will still be shown

// Call & return (nullable)
#define CRN(INST, METHOD, ...)  ({ auto tmp_ = (INST); (tmp_ ? tmp_->METHOD(__VA_ARGS__) : nullptr); })

// Create, call & return (nullable)
#define CCRN(CLS, METHOD, ...)  ({ CLS *tmp_ = CLS::create(); auto result_ = (tmp_ ? tmp_->METHOD(__VA_ARGS__) : nullptr); if (tmp_ && !result_) tmp_->dtor(); result_; })


// TODO: Throw?
#define REQUIRE(STMT)                                   \
    if (!(STMT)) {                                      \
        ERR("Requirement not satisfied: %s\n", #STMT);  \
        abort();                                        \
    }


extern int verbosity;

void err_(const char *funcName, int lineNo, const char *msg, ...);


#endif // GENERAL_H_GUARD

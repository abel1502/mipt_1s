#ifndef GENERAL_H_GUARD
#define GENERAL_H_GUARD


#define ERR(msg, ...) err_(__func__, __LINE__, msg, ##__VA_ARGS__)


#define FACTORIES(CLS)          \
    static CLS *create() {      \
        /* Auto-throws error */ \
        CLS *self = new CLS();  \
                                \
        return self->ctor();    \
    }                           \
                                \
    void destroy() {            \
        dtor();                 \
        delete this;            \
    }


extern int verbosity;

void err_(const char *funcName, int lineNo, const char *msg, ...);


#endif // GENERAL_H_GUARD

#ifndef GENERAL_H_GUARD
#define GENERAL_H_GUARD


#ifndef __cplusplus
typedef enum { false, true } bool;
#endif


#define ERR(msg, ...) err_(__func__, __LINE__, msg, ##__VA_ARGS__)


// Usage: #define ARGTYPE_CASE_(NAME_CAP, NAME_LOW, TYPE, FMT_U, FMT_S)
#define ARGTYPE_SWITCH_(DEFAULT) \
    switch (opcode->addrMode.type) { \
        ARGTYPE_CASE_(DF, df, double, "%lg", "%lg") \
        ARGTYPE_CASE_(FL, fl, float, "%g", "%g") \
        ARGTYPE_CASE_(FH, fh, float, "%g", "%g") \
        ARGTYPE_CASE_(QW, qw, uint64_t, "%llu", "%lld") \
        ARGTYPE_CASE_(DWL, dwl, uint32_t, "%u", "%d") \
        ARGTYPE_CASE_(DWH, dwh, uint32_t, "%u", "%d") \
        ARGTYPE_CASE_(WL, wl, uint16_t, "%hu", "%hd") \
        ARGTYPE_CASE_(WH, wh, uint16_t, "%hu", "%hd") \
        ARGTYPE_CASE_(BL, bl, uint8_t, "%hhu", "%hhd") \
        ARGTYPE_CASE_(BH, bh, uint8_t, "%hhu", "%hhd") \
    default: \
        DEFAULT \
    }


void err_(const char *funcName, int lineNo, const char *msg, ...);


#endif // GENERAL_H_GUARD

#ifndef GENERAL_H_GUARD
#define GENERAL_H_GUARD


#ifndef __cplusplus
typedef enum { false, true } bool;
#endif


#define ERR(msg, ...) err_(__func__, __LINE__, msg, ##__VA_ARGS__)


void err_(const char *funcName, int lineNo, const char *msg, ...);


#endif // GENERAL_H_GUARD

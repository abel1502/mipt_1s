#ifndef GENERAL_H_GUARD
#define GENERAL_H_GUARD

#ifdef NDEBUG
#define ERR(msg, ...)
#else
#define ERR(msg, ...) do {fprintf(stderr, "[ERROR in %s() on #%d] " msg "\n", __func__, __LINE__, ##__VA_ARGS__); /*perror(">");*/} while (0)
#endif

#ifndef __cplusplus
typedef enum { false, true } bool;
#endif

#endif // GENERAL_H_GUARD

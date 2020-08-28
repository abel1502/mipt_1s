/**
 * This is (kind of) a framework for unit testing in C.
 *
 * Basic usage:
 *   Define TEST to enable unit testing in general.
 *   Then in main(), embed the following code:
 *
 *   TEST_MAIN(
 *       <Here you can place all your tests' code>
 *       TEST_EXIT_LABEL
 *       <Here you can place all your cleanup code>
 *   )
 *
 *
 */  // I have no idea if doxygen can document macros, so why not make a module-wise docstring?)

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include <setjmp.h>


#define MACROFUNC(...) do {__VA_ARGS__} while (0)


#ifdef TEST

jmp_buf __test_env;
jmp_buf __cleanup_env;

#define TEST_MAIN(testcode, cleanupcode) \
    if (setjmp(__test_env) != 0) { \
        testcode \
        longjmp(__cleanup_env, 1); \
    } \
    if (setjmp(__cleanup_env) != 0) { \
        cleanupcode \
    } else { \
        longjmp(__test_env, 1); \
    } \
    return EXIT_SUCCESS;
#else
#define TEST_MAIN(testcode, cleanupcode)
#endif


#define TEST_EXIT() MACROFUNC(longjmp(__cleanup_env, 1);)
#define TEST_MSG(msg, ...) MACROFUNC(printf("[TEST] " msg "\n", ##__VA_ARGS__);)
#define TEST_ASSERT(stmt) MACROFUNC(if (!(stmt)) {TEST_MSG("Assertion (" #stmt ") failed"); TEST_EXIT();})
#define TEST_ASSERT_M(stmt, msg, ...) MACROFUNC(if (!(stmt)) {TEST_MSG("Assertion (" #stmt ") failed with message " #msg, ##__VA_ARGS__); TEST_EXIT();})


const double _EPSILON = 1e-8;

typedef enum cmp_res {
    CMP_LESS,
    CMP_EQUAL,
    CMP_GREATER
} cmp_res_t;

/**
 * Helper function to approximately compare doubles
 *
 * @param [in] left  The first value
 * @param [in] right The second value
 */
cmp_res_t cmpDouble(double left, double right) {
    double ldelta = fabs(left * _EPSILON);
    double rdelta = fabs(right * _EPSILON);
    double delta = (ldelta < rdelta) ? ldelta : rdelta;
    if (left < right - delta)
        return CMP_LESS;
    else if (right - delta <= left && left <= right + delta)
        return CMP_EQUAL;
    else
        return CMP_GREATER;
}

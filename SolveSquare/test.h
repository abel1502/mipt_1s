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

#define TEST_EXIT() MACROFUNC(longjmp(__cleanup_env, 1);)

#define TEST_MSG(msg) MACROFUNC(printf("[TEST] " msg "\n");)

#define TEST_ASSERT(stmt) MACROFUNC(if (!(stmt)) {TEST_MSG("Assertion (" #stmt ") failed"); TEST_EXIT();})

#define TEST_ASSERT_M(stmt, msg) MACROFUNC(if (!(stmt)) {TEST_MSG("Assertion (" #stmt ") failed with message " #msg); TEST_EXIT();})

#else

#define TEST_MAIN(...)
#define TEST_EXIT_LABEL
#define TEST_EXIT()
#define TEST_MSG(msg)
#define TEST_ASSERT(stmt)
#define TEST_ASSERT_M(stmt, msg)

#endif






/**
 * @file test.h
 * This is (kind of) a framework for unit testing in C.
 *
 * Basic usage:
 *   Define TEST to enable unit testing in general.
 *   Then in main(), embed the following code:
 *
 *   TEST_MAIN(
 *       <Here you can place all yout setup code>
 *       ,
 *       <Here you can place all your tests' code>
 *       ,
 *       <Here you can place all your cleanup code>
 *   )
 *
 *
 */  // I have no idea if doxygen can document macros, so why not make a module-wise docstring?)


#ifndef TESTLIB_GUARD
#define TESTLIB_GUARD

#ifdef TEST

#include "TXLib.h"

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include <setjmp.h>
#include <signal.h>


#define MACROFUNC(...) do {__VA_ARGS__} while (0)


static jmp_buf __test_env;
static jmp_buf __cleanup_env;

#define TEST_MAIN(setupcode, testcode, cleanupcode) \
    setupcode \
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

#define TEST_EXIT()         MACROFUNC(longjmp(__cleanup_env, 1);)

#define TEST_MSG(msg, ...)  MACROFUNC(printf("[TEST (%s#%d)] " msg "\n", __FILE__, __LINE__, ##__VA_ARGS__);)

#define TEST_SETUP(stmt)    MACROFUNC(TEST_MSG("Setting up {%s}", #stmt); stmt)

#define TEST_ASSERT(stmt)   MACROFUNC( \
    if (!(stmt)) { \
        $r; TEST_MSG("[!] Assertion (%s) failed", #stmt); $d; TEST_EXIT(); \
    } else { \
        printf("."); \
    } \
)

#define TEST_ASSERT_M(stmt, msg, ...) MACROFUNC( \
    if (!(stmt)) { \
        $r; TEST_MSG("[!] Assertion (%s) failed with message " #msg, #stmt, ##__VA_ARGS__); $d; TEST_EXIT(); \
    } else { \
        printf("."); \
    } \
)

#else
#define TEST_MAIN(setupcode, testcode, cleanupcode)
#endif // TEST

#endif // TESTLIB_GUARD

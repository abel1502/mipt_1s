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


#ifndef MACROFUNC
#define MACROFUNC(...) do {__VA_ARGS__} while (0)
#endif


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

#define TEST_PMSG(msg, ...)   MACROFUNC(printf(msg, ##__VA_ARGS__);)
#define TEST_PSMSG(msg, ...)  MACROFUNC($g; TEST_PMSG(msg, ##__VA_ARGS__); $d;)
#define TEST_PFMSG(msg, ...)  MACROFUNC($r; TEST_PMSG(msg, ##__VA_ARGS__); $d;)

#define TEST_MSG(msg, ...)   MACROFUNC(printf("[TEST (%s#%d)] " msg "\n", __FILE__, __LINE__, ##__VA_ARGS__);)
#define TEST_SMSG(msg, ...)  MACROFUNC($g; TEST_MSG(msg, ##__VA_ARGS__); $d;)
#define TEST_FMSG(msg, ...)  MACROFUNC($r; TEST_MSG(msg, ##__VA_ARGS__); $d;)

#define TEST_SETUP(stmt)    MACROFUNC(TEST_MSG("Setting up {%s}", #stmt); stmt)

#define TEST_ASSERT(stmt)   MACROFUNC( \
    if (!(stmt)) { \
        TEST_FMSG("[!] Assertion (%s) failed", #stmt); TEST_EXIT(); \
    } else { \
        TEST_PMSG("."); \
    } \
)

#define TEST_ASSERT_M(stmt, msg, ...) MACROFUNC( \
    if (!(stmt)) { \
        TEST_FMSG("[!] Assertion (%s) failed with message " #msg, #stmt, ##__VA_ARGS__); TEST_EXIT(); \
    } else { \
        TEST_PMSG("."); \
    } \
)

#else
#define TEST_MAIN(setupcode, testcode, cleanupcode)
#endif // TEST

#endif // TESTLIB_GUARD

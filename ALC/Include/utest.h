/**
 * @file test.h
 * This is (kind of) a framework for unit testing in C.
 *
 * Basic usage:
 *   Define UTEST to enable unit testing in general.
 *
 *   @warning Whether you're using a separate build configuration or not, you HAVE TO place
 *   the UTEST_MAIN macro in the beginning of your main function, before the actual code
 *
 * Macros:
 *  - **UTEST_MAIN**(SETUP_CODE, TEST_CODE, CLEANUP_CODE) @n
 *    The main unit test container. Must be placed in main.
 *    Will either do nothing when UTEST isn't defined, or replace main's code
 *    with its contents in the other case.
 *
 *    @warning Only one of these may be present per translation unit
 *    (i.e. cpp file and everything it includes) - disobedience is considered UB
 *
 *  - **UTEST_IF_TEST**(CODE) @n
 *    Only execute CODE if UTEST is defined
 *
 *  - **UTEST_IF_NTEST**(CODE) @n
 *    The opposite
 *
 *  - *(From this point everything is defined only when UTEST is, so be careful to wrap it into sufficient checks)*
 *
 *  - **UTEST_EXIT()**
 *    Skip to cleanup.
 *
 *  - **UTEST_PMSG**(MSG, ...) @n
 *    Print a plain message (equivalent to printf).
 *
 *  - **UTEST_PSMSG**(MSG, ...) @n
 *    Print a plain success message.
 *
 *  - **UTEST_PFMSG**(MSG, ...) @n
 *    Print a plain failure message.
 *
 *  - **UTEST_MSG**(MSG, ...) @n
 *    Print a headered message.
 *
 *  - **UTEST_SMSG**(MSG, ...) @n
 *    Print a headered success message.
 *
 *  - **UTEST_FMSG**(MSG, ...) @n
 *    Print a headered failure message.
 *
 *  - **UTEST_SETUP**(STMT) @n
 *    Log an expression with a header and execute it.
 *
 *  - **UTEST_ASSERT**(STMT) @n
 *    Make a test assertion, and either fail with a generic message (including STMT) if STMT is false, or print '.' in white and continue.
 *
 *  - **UTEST_ASSERT_M**(STMT) @n
 *    Same as UTEST_ASSERT, but prints a custom error message alongside the generic one.
 *
 */

#ifndef ALC_UTEST_H
#define ALC_UTEST_H

#ifdef ALC_UTEST_TXCOLORS
#include <TXLib.h>
#else
#define $g
#define $r
#endif

#include <cstdio>
#include <csetjmp>
#include "macros.h"


#ifdef UTEST

static jmp_buf UTEST_test_env_;
static jmp_buf UTEST_cleanup_env_;

#define UTEST_MAIN(SETUP_CODE, TEST_CODE, CLEANUP_CODE) \
    SETUP_CODE \
    if (setjmp(UTEST_test_env_) != 0) { \
        TEST_CODE \
        longjmp(UTEST_cleanup_env_, 1); \
    } \
    if (setjmp(UTEST_cleanup_env_) != 0) { \
        CLEANUP_CODE \
    } else { \
        longjmp(UTEST_test_env_, 1); \
    } \
    return EXIT_SUCCESS;

#define UTEST_IF_TEST (CODE)  CODE
#define UTEST_IF_NTEST(CODE)

#define UTEST_EXIT()         MACROFUNC(longjmp(UTEST_cleanup_env_, 1);)

#define UTEST_PMSG(MSG, ...)   MACROFUNC(printf(MSG, ##__VA_ARGS__);)
#define UTEST_PSMSG(MSG, ...)  MACROFUNC($g; UTEST_PMSG(MSG, ##__VA_ARGS__); $d;)
#define UTEST_PFMSG(MSG, ...)  MACROFUNC($r; UTEST_PMSG(MSG, ##__VA_ARGS__); $d;)

#define UTEST_MSG(MSG, ...)   MACROFUNC(printf("[UTEST (%s#%d)] " MSG "\n", __FILE__, __LINE__, ##__VA_ARGS__);)
#define UTEST_SMSG(MSG, ...)  MACROFUNC($g; UTEST_MSG(MSG, ##__VA_ARGS__); $d;)
#define UTEST_FMSG(MSG, ...)  MACROFUNC($r; UTEST_MSG(MSG, ##__VA_ARGS__); $d;)

#define UTEST_SETUP(STMT)    MACROFUNC(UTEST_MSG("Setting up {%s}", #STMT); STMT)

#define UTEST_ASSERT(STMT)   MACROFUNC( \
    if (!(STMT)) { \
        UTEST_FMSG("[!] Assertion (%s) failed", #STMT); UTEST_EXIT(); \
    } else { \
        UTEST_PMSG("."); \
    } \
)

#define UTEST_ASSERT_M(STMT, MSG, ...) MACROFUNC( \
    if (!(STMT)) { \
        UTEST_FMSG("[!] Assertion (%s) failed with message " #MSG, #STMT, ##__VA_ARGS__); UTEST_EXIT(); \
    } else { \
        UTEST_PMSG("."); \
    } \
)

#else

#define UTEST_MAIN(SETUP_CODE, TEST_CODE, CLEANUP_CODE)

#define UTEST_IF_TEST (CODE)
#define UTEST_IF_NTEST(CODE)  CODE

#endif // UTEST


#ifndef ALC_UTEST_TXCOLORS
#undef $g
#undef $r
#endif

#endif // ALC_UTEST_H

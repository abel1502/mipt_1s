#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <assert.h>


#ifdef TEST
#define TEST_MAIN(...) __VA_ARGS__ return EXIT_SUCCESS;
#else
#define TEST_MAIN(...)
#endif


#define TEST_ASSERT(stmt) if (!(stmt)) {printf("Test assertion (" #stmt ") failed");}

//#define TEST
#include "../libs/test.h"

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <stdbool.h>


#define ERR(msg, ...) do {if (verbose) fprintf(stderr, "[ERROR in %s()] " msg "\n", __func__, ##__VA_ARGS__);} while (0)

//--------------------------------------------------------------------------------

static const double EPSILON = 1e-8;

bool verbose = false;

//--------------------------------------------------------------------------------

/**
 * Possible solution types
 */
typedef enum se_type {
    SE_NO_ROOTS,  /**< No real roots */
    SE_ONE_ROOT,  /**< 1 real root */
    SE_TWO_ROOTS, /**< 2 real roots */
    SE_ANY_ROOT   /**< Any number is a root */
} se_type_t;

/**
 * Solution information
 */
typedef struct se_solution {
    double x1;      /**< The first solution (if present) */
    double x2;      /**< The second solution (if present) */
    se_type_t type; /**< The solution type */
} se_solution_t;

/**
 * Solution constructor, but in C
 *
 * @param [in] res  The actual solution instance
 * @param [in] type res.type
 * @param [in] x1   res.x1
 * @param [in] x2   res.x2
 */
se_solution_t *initSolution(se_solution_t *res, se_type type, double x1, double x2);

/**
 * Pretty-prints a solution into the console
 *
 * @param [in] solution The solution to print
 *
 * @return Error code (0 means success, non-0 - an exception)
 */
int logSolution(se_solution_t *solution);

/**
 * Shows a constant banner at the beginning of the execution
 */
void showBanner(void);

/**
 * Shows a usage help message
 *
 * @param [in] binname The current binary's name (argv[0])
 */
void showHelp(char *binname);

/**
 * Tries to parse the equation coefficients from the program's arguments
 *
 * @param [in]  argc  Argument count
 * @param [in]  argv  Argument values
 * @param [out] a     `a` coefficient
 * @param [out] b     `b` coefficient
 * @param [out] c     `c` coefficient
 *
 * @return Error code (0 means success, non-0 - an exception)
 */
int parseArgCoeffs(int argc, char **argv, double *a, double *b, double *c);

/**
 * Solves a square equation (a*x^2+b*x+c==0)
 *
 * @param [in]  a        `a` coefficient
 * @param [in]  b        `b` coefficient
 * @param [in]  c        `c` coefficient
 * @param [out] solution Pointer to the solution struct
 *
 * @return Error code (0 means success, non-0 - an exception)
 */
int solveSE(double a, double b, double c, se_solution_t *solution);

/**
 * Helper function to approximately compare doubles
 *
 * @param [in] left  The first value
 * @param [in] right The second value
 *
 * @return -1 if left < right, 0 if left == right, 1 if left > right
 */
int cmpDouble(double left, double right);

#ifdef TEST

void test_solveSE_(double a, double b, double c, int exp_ret, se_solution_t *exp_solution);

void test_solveSE(void);

#endif

//================================================================================

int main(int argc, char **argv) {
    TEST_MAIN(
        verbose = true;
        ,
        test_solveSE();
        $g; TEST_MSG("Passed All."); $d;
        ,
    )

    showBanner();

    int opt;
    while ((opt = getopt(argc, argv, "v")) != -1) {
        switch (opt) {  // For scalability
        case 'v':
            verbose = true;
            break;
        default:
            ERR("Bad args");
            showHelp(argv[0]);
            return EXIT_FAILURE;
        }
    }

    double a = 0, b = 0, c = 0;
    if (parseArgCoeffs(argc, argv, &a, &b, &c) != 0) {
        ERR("Bad args");
        showHelp(argv[0]);
        return EXIT_FAILURE;
    }

//    printf("Enter the coefficients: ");
//    if (scanf("%lg %lg %lg", &a, &b, &c) != 3) {
//        ERR("Error while trying to read input");
//        return EXIT_FAILURE;  // Since I'm only allowed to use exits in main, there's no more consistency requirement between all functions and I can just use return
//    }
    printf("Solving %lg*x^2 + %lg*x + %lg == 0\n\n", a, b, c);

    se_solution_t solution;
    if (solveSE(a, b, c, &solution) != 0) {
        ERR("Error while solving the equation");
        return EXIT_FAILURE;
    }

    if (logSolution(&solution) != 0) {
        ERR("Error while printing the solution");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

//================================================================================

int solveSE(double a, double b, double c, se_solution_t *solution) {
    if (!isfinite(a) || !isfinite(b) || !isfinite(c)) {
        ERR("some coefficient isn\'t a finite number");
        return 1;
    }

//    if (solution == NULL) {
//        ERR("nullptr solution");
//        return 2;
//    }
    assert(solution != NULL);

    if (cmpDouble(a, 0) == 0) {
        if (cmpDouble(b, 0) == 0) {
            if (cmpDouble(c, 0) == 0) {
                solution->type = SE_ANY_ROOT;
            } else {
                solution->type = SE_NO_ROOTS;
            }
        } else {
            solution->type = SE_ONE_ROOT;
            solution->x1 = -c / b;
        }
    } else {
        double discr = b * b - 4 * a * c;

        int discr_0 = cmpDouble(discr, 0);

        if (discr_0 < 0) {
            solution->type = SE_NO_ROOTS;
        } else {
            if (discr_0 == 0) {
                solution->type = SE_ONE_ROOT;
            } else {
                solution->type = SE_TWO_ROOTS;
            }
            double sqrt_discr = sqrt(discr);
            solution->x1 = (-b - sqrt_discr) / (2 * a);
            solution->x2 = (-b + sqrt_discr) / (2 * a);
        }
    }
    return 0;
}

//--------------------------------------------------------------------------------

int cmpDouble(double left, double right) {
    double ldelta = fabs(left * EPSILON);
    double rdelta = fabs(right * EPSILON);
    double delta = (ldelta < rdelta) ? ldelta : rdelta;
    if (left < right - delta)
        return -1;
    else if (right - delta <= left && left <= right + delta)
        return 0;
    else
        return 1;
}

//--------------------------------------------------------------------------------

se_solution_t *initSolution(se_solution_t *res, se_type type, double x1, double x2) {
    res->type = type;
    res->x1 = x1;
    res->x2 = x2;
    return res;
}

//--------------------------------------------------------------------------------

int logSolution(se_solution_t *solution) {
//    if (solution == NULL) {
//        ERR("nullptr solution");
//        return 1;
//    }
    assert(solution != NULL);

    switch (solution->type) {
    case SE_NO_ROOTS:
        printf("No solutions\n");
        break;
    case SE_ONE_ROOT:
        printf("x = %lg\n", solution->x1);
        break;
    case SE_TWO_ROOTS:
        printf("x1 = %lg\nx2 = %lg\n", solution->x1, solution->x2);
        break;
    case SE_ANY_ROOT:
        printf("x = Anything\n");
        break;
    default:
        ERR("Unexpected value for solution type: %d", solution->type);
        return 2;
    }
    return 0;
}

//--------------------------------------------------------------------------------

void showBanner(void) {
    printf("########################\n"
           "#                      #\n"
           "#     Solve Square     #\n"
           "#    (c) Abel, 2020    #\n"
           "#                      #\n"
           "########################\n");

    printf("This program solves square equations in the following form:\n"
           "a * x^2 + b * x + c == 0\n"
           "with variable coefficients\n\n");
}

//--------------------------------------------------------------------------------

void showHelp(char *binname) {
    printf("\nUsage: %s a b c [-v]\n\n"
           "a, b and c are the coefficients for the equation.\n"
           "-v option enables verbose error output\n\n", binname);
}

//--------------------------------------------------------------------------------

int parseArgCoeffs(int argc, char **argv, double *a, double *b, double *c) {
    if (argc < 4) {
        ERR("Too few arguments: %d", argc);
        return 1;
    }

    if ((sscanf(argv[1], "%lg", a) < 1) || (sscanf(argv[2], "%lg", b) < 1) || (sscanf(argv[3], "%lg", c) < 1)) {
        ERR("Bad argument format");
        return 2;
    }

    return 0;
}

//--------------------------------------------------------------------------------

#ifdef TEST
void test_solveSE_(double a, double b, double c, int exp_ret, se_solution_t *exp_solution) {
    se_solution_t solution;

    TEST_MSG("Testing solve_SE on a=%lg, b=%lg, c=%lg", a, b, c);
    int ret = solveSE(a, b, c, &solution);

    TEST_ASSERT_M(ret == exp_ret, "Unexpected return value: %d instead of %d", ret, exp_ret);

    if (exp_ret == 0) {
        TEST_ASSERT_M(solution.type == exp_solution->type, "Different numbers of roots: %d instead of %d", solution.type, exp_solution->type);

        if (solution.type == SE_ONE_ROOT || solution.type == SE_TWO_ROOTS) {
            TEST_ASSERT_M(cmpDouble(solution.x1, exp_solution->x1) == 0, "Different 1st roots: %lg != %lg", solution.x1, exp_solution->x1);
        }

        if (solution.type == SE_TWO_ROOTS) {
            TEST_ASSERT_M(cmpDouble(solution.x2, exp_solution->x2) == 0, "Different 2nd roots: %lg != %lg", solution.x2, exp_solution->x2);
        }
    }

    $g; TEST_MSG("Passed"); $d;
}

//--------------------------------------------------------------------------------

void test_solveSE(void) {
    se_solution_t exp_solution;
    test_solveSE_(1, 2, 1, 0, initSolution(&exp_solution, SE_ONE_ROOT, -1, 0));
    test_solveSE_(1, 4, 4, 0, initSolution(&exp_solution, SE_ONE_ROOT, -2, 0));
    test_solveSE_(0, 1, 1, 0, initSolution(&exp_solution, SE_ONE_ROOT, -1, 0));
    test_solveSE_(0, 0, 0, 0, initSolution(&exp_solution, SE_ANY_ROOT, 0, 0));
    test_solveSE_(0, 0, 1, 0, initSolution(&exp_solution, SE_NO_ROOTS, 0, 0));
    test_solveSE_(0.1, -0.5, 0.6, 0, initSolution(&exp_solution, SE_TWO_ROOTS, 2, 3));
    test_solveSE_(NAN, 0, 0, 1, &exp_solution);
}
#endif // TEST


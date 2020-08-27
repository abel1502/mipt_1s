#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#if !NDEBUG  // I'm creating this extra flag on purpose because I may change my ming about when to log errors or when not to
#define SHOWERRORS
#endif

#ifdef SHOWERRORS
#define ERR(source, msg, ...) printf("ERROR in " source "(): " msg, ##__VA_ARGS__);
#else
#define ERR(source, msg, ...)
#endif


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
 * Shows a constant banner at the beginning of the execution
 */
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


/**
 * Pretty-prints a solution into the console
 *
 * @param solution The solution to print
 *
 * @return Error code (0 means success, non-0 - an exception)
 */
int logSolution(se_solution_t * solution) {
    if (solution == NULL) {
        ERR("logSolution", "nullptr solution");
        return 1;
    }
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
        ERR("logSolution", "Unexpected value for solution type: %d\n", solution->type);
        return 2;
    }
    return 0;
}


/**
 * Solves a square equation (a*x^2+b*x+c==0)
 *
 * @param [in]  a        `a` coefficient
 * @param [in]  b        `b` coefficient
 * @param [in]  c        `c` coefficient
 * @param [out] solution Pointer to the solution struct
 *
 * @return Error value. (0 means success, non-0 indicates an exception)
 */
int solveSE(double a, double b, double c, se_solution_t * solution) {
    if (!std::isfinite(a) || !std::isfinite(b) || !std::isfinite(c)) {
        ERR("solveSE", "some coefficient isn\'t a finite number");
        return 1;
    }
    if (solution == NULL) {
        ERR("solveSE", "nullptr solution");
        return 2;
    }

    if (a == 0) {
        if (b == 0) {
            if (c == 0) {
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
        if (discr < 0) {
            solution->type = SE_NO_ROOTS;
        } else {
            if (discr == 0) {
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


int main()
{
    showBanner();

    double a = 0, b = 0, c = 0;
    printf("Enter the coefficients: ");
    if (scanf("%lg %lg %lg", &a, &b, &c) != 3) {
        ERR("main", "Error while trying to read input");
        return 1;  // Since I'm only allowed to use exits in main, there's no more consistency requirement between all functions and I can just use return
    }

    se_solution_t solution;
    if (solveSE(a, b, c, &solution) != 0) {
        ERR("main", "Error while solving the equation");
        return 1;
    }

    if (logSolution(&solution) != 0) {
        ERR("main", "Error while printing the solution");
        return 1;
    }
    return 0;
}


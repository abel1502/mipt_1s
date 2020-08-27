#include <stdio.h>
#include <math.h>
#include <stdlib.h>


const char SE_ANY_X = -1;


typedef struct se_solution {
    signed char nRoots;
    double x1;
    double x2;
} se_solution_t;


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


void logSolution(se_solution_t * solution) {
    switch (solution->nRoots) {
    case 0:
        printf("No solutions\n");
        break;
    case 1:
        printf("x = %lg\n", solution->x1);
        break;
    case 2:
        printf("x1 = %lg\nx2 = %lg\n", solution->x1, solution->x2);
        break;
    case SE_ANY_X:
        printf("x = Anything\n");
        break;
    default:
        printf("ERR: Unexpected value for nRoots: %d\n", solution->nRoots);
        exit(1);
    }
}


int solveSE(double a, double b, double c, se_solution_t * solution) {
    // TODO: Validity checks
    if (a == 0) {
        if (b == 0) {
            if (c == 0) {
                solution->nRoots = SE_ANY_X;
            } else {
                solution->nRoots = 0;
            }
        } else {
            solution->nRoots = 1;
            solution->x1 = -c / b;
        }
    } else {
        double discr = b * b - 4 * a * c;
        if (discr < 0) {
            solution->nRoots = 0;
        } else {
            if (discr == 0) {
                solution->nRoots = 1;
            } else {
                solution->nRoots = 2;
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
        printf("ERR: Error while trying to read input");
        exit(1);
    }

    se_solution_t solution;
    int result = 0;
    if ((result = solveSE(a, b, c, &solution)) != 0) {
        printf("ERR: Error #%d while solving the equation", result);
        exit(1);
    }

    logSolution(&solution);
    return 0;
}

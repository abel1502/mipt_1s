#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <getopt.h>
#include <cstring>

#include "general.h"
#include "expr.h"


using namespace SymbolDiff;


static void showHelp(const char *binName) {
    printf("This program can differentiate expressions and produce latex reports on the process.\n"
           "Usage:  %s [-h] [-v] -i ifile [-b by]\n"
           "  -h        - show this and exit\n"
           "  -v        - increase verbosity\n"
           "  -i ifile  - the name of the input file containing the expression to differentiate\n"
           "  -b by     - the variable to differentiate by (single character!), x by default\n"
           "\n", binName);
}


int main(int argc, char **argv) {
    FILE *ifile = nullptr;
    char by = 'x';

    int c = 0;

    while ((c = getopt(argc, argv, "+i:b:vh")) != -1) {
        switch (c) {
        case 'h':
            if (ifile)  fclose(ifile);

            showHelp(argv[0]);
            return 1;
        case 'i':
            ifile = fopen(optarg, "rb");

            if (!ifile) {
                ERR("Couldn't open %s to read", optarg);
                return 2;
            }

            break;
        case 'b':
            if (strlen(optarg) != 1) {
                ERR("Variable name must contain exactly 1 character");
                return 1;
            }

            by = optarg[0];

            break;
        case 'v':
            verbosity++;
            break;
        case '?':
            if (ifile)  fclose(ifile);

            ERR("Unknown option: -%c.", optopt);
            showHelp(argv[0]);
            return 1;
        default:
            ERR("Shouldn't be reachable");
            abort();
        }
    }

    if (!ifile) {
        ERR("ifile is required.");
        return 1;
    }

    if (optind != argc) {
        fclose(ifile);

        ERR("Unexpected number of positional arguments.");
        showHelp(argv[0]);
        return 1;
    }

    ExprTree expr = {};
    expr.ctor();

    if (!expr.read(ifile)) {
        fclose(ifile);

        expr.dtor();

        ERR("Failed to read the expression from the input file");
        return 3;
    }

    fclose(ifile);

    if (verbosity >= 2) {
        printf("Raw input: ");
        expr.dump();
    }

    expr.simplify();

    if (verbosity >= 2) {
        printf("Simplified input: ");
        expr.dump();
    }

    ExprTree *deriv = expr.diff(by);

    if (verbosity >= 2) {
        printf("Derivative: ");
        deriv->dump();
    }

    /*deriv->simplify();

    if (verbosity >= 2) {
        printf("Simplified derivative: ");
        deriv->dump();
    }*/

    printf("Done.\n");

    expr.dtor();
    deriv->dtor();


    return 0;
}

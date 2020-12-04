#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <getopt.h>

#include "general.h"
#include "expr.h"


using namespace SymbolDiff;


static void showHelp(const char *binName) {
    printf("This program can differentiate expressions and produce latex reports on the process.\n"
           "Usage:  %s [-h] [-v] -i ifile -o ofile\n"
           "  -h        - show this and exit\n"
           "  -v        - increase verbosity\n"
           "  -i ifile  - the name of the input file containing the expression to differentiate\n"
           "  -o ofile  - the name of the output file\n"
           "\n", binName);
}


int main(int argc, char **argv) {
    /*verbosity = 3;

    ExprNode *tmp = ExprNode::read("( (1) + ( sin((-(2)) * ((x) ^ ((15) * (y)))) ) )");

    if (tmp) {
        printf("$$ ");
        VCALL(tmp, writeTex, stdout);
        printf(" $$\n");

        VCALL(tmp, dump);
        printf("\n");

        ExprNode *diff = VCALL(tmp, diff, 'x');

        VCALL(diff, dump);
        printf("\n");

        bool wasTrivial = false;
        while (!wasTrivial)
            diff = VCALL(diff, simplify, &wasTrivial);

        printf("$$ ");
        VCALL(diff, writeTex, stdout);
        printf(" $$\n");

        VCALL(diff, dump);
        printf("\n");

        diff->destroy();
        tmp->destroy();
    } else {
        printf("Corrupt expression, sorry(...\n");
    }


    return 0;*/


    FILE *ifile = nullptr;

    int c = 0;

    while ((c = getopt(argc, argv, "+i:vh")) != -1) {
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

    ExprTree *deriv = expr.diff();

    if (verbosity >= 2) {
        printf("Derivative: ");
        deriv->dump();
    }

    /*deriv->simplify();

    if (verbosity >= 2) {
        printf("Simplified derivative: ");
        deriv->dump();
    }*/

    expr.dtor();
    deriv->dtor();


    return 0;
}

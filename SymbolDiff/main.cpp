#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <getopt.h>

#include "general.h"
#include "expr.h"


using namespace SymbolDiff;


static void showHelp(const char *binName) {
    printf("This program can differentiate expressions and produce latex reports on the process.\n"
           "Usage:  %s [-h] [-v] [-i ifile] [-o ofile]\n"
           "  -h        - show this and exit\n"
           "  -v        - increase verbosity\n"
           "  -i ifile  - the name of the input file containing the expression to differentiate\n"
           "              (if not specified, stdin is used)\n"
           "  -o ofile  - the name of the output file\n"
           "              (if not specified, stdout is used).\n"
           "\n", binName);
}


int main(int argc, char **argv) {
    ExprNode *one = ExprNode::create()->ctorConst(123);
    ExprNode *two = ExprNode::create()->ctorVar('x');
    ExprNode *add = ExprNode::create()->ctorBinOp(BinOp_Add, one, two);

    VCALL(add, dump);

    add->dtor();
    delete add;


    return 0;


    FILE *ifile = stdin;
    FILE *ofile = stdout;

    int c = 0;

    while ((c = getopt(argc, argv, "+i:o:vh")) != -1) {
        switch (c) {
        case 'h':
            showHelp(argv[0]);
            return 1;
        case 'i':
            ifile = fopen(optarg, "rb");

            if (ifile == NULL) {
                ERR("Couldn't open %s to read", optarg);
                return 2;
            }

            break;
        case 'o':
            ofile = fopen(optarg, "wb");

            if (ofile == NULL) {
                ERR("Couldn't open %s to write", optarg);
                return 2;
            }

            break;
        case 'v':
            verbosity++;
            break;
        case '?':
            ERR("Unknown option: -%c.", optopt);
            showHelp(argv[0]);
            return 1;
        default:
            ERR("Shouldn't be reachable");
            abort();
        }
    }

    if (optind != argc) {
        fclose(ifile);
        fclose(ofile);

        ERR("Unexpected number of positional arguments.");
        showHelp(argv[0]);
        return 1;
    }

    // TODO

    return 0;
}

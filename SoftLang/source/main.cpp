#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <getopt.h>

#include "general.h"


//using namespace SoftLang;


static void showBanner() {
    printf("#####################################\n"
           "#                                   #\n"
           "#    Abel Source Format Compiler    #\n"
           "#          (c) Abel, 2020           #\n"
           "#                                   #\n"
           "#####################################\n"
           "\n"
           "This program assembles an .asf file into an .aaf file.\n\n");
}

static void showHelp(const char *binName) {
    printf("Usage:  %s [-h] [-v] -i ifile -o ofile\n"
           "  -h        - show this and exit\n"
           "  -v        - increase verbosity\n"
           "  -i ifile  - name of the input file\n"
           "  -o ofile  - name of the output file\n"
           "\n", binName);
}


int main(int argc, char **argv) {
    FILE *ifile = nullptr;
    FILE *ofile = nullptr;

    #define CLEANUP_            \
        if (ifile) {            \
            fclose(ifile);      \
            ifile = nullptr;    \
        }                       \
        if (ofile) {            \
            fclose(ofile);      \
            ofile = nullptr;    \
        }

    showBanner();

    int c = 0;

    while ((c = getopt(argc, argv, "+i:o:vh")) != -1) {
        switch (c) {
        case 'h':
            CLEANUP_

            showHelp(argv[0]);
            return 0;
        case 'i':
            ifile = fopen(optarg, "r");

            if (!ifile) {
                CLEANUP_

                ERR("Couldn't open %s to read", optarg);
                return 2;
            }

            break;
        case 'o':
            ofile = fopen(optarg, "w");

            if (!ofile) {
                CLEANUP_

                ERR("Couldn't open %s to read", optarg);
                return 2;
            }

            break;
        case 'v':
            verbosity++;
            break;
        case '?':
            CLEANUP_

            ERR("Unknown option: -%c.", optopt);
            showHelp(argv[0]);
            return 1;
        default:
            CLEANUP_

            ERR("Shouldn't be reachable");
            abort();
        }
    }

    if (!ifile || !ofile) {
        CLEANUP_

        ERR("Both ifile and ofile are required");
        return 1;
    }

    if (optind != argc) {
        CLEANUP_

        ERR("Unexpected number of positional arguments.");
        showHelp(argv[0]);
        return 1;
    }

    // TODO

    CLEANUP_

    return 0;

    #undef CLEANUP_
}

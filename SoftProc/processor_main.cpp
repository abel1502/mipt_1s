#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
//#include <unistd.h>
#include <getopt.h>

// To avoid redundant redeclarations
#define STACK_NOIMPL
#define CHECKSUM_NOIMPL

#include "constants.h"
#include "aef_file.h"
#include "program.h"
#include "general.h"


void showBanner();

void showHelp(char *binName);


int main(int argc, char **argv) {
    showBanner();

    FILE *ifile = NULL;
    extern int verbosity;

    int c = 0;

    while ((c = getopt(argc, argv, "vh")) != -1) {
        switch (c) {
        case 'h':
            showHelp(argv[0]);
            return 1;  // TODO: Encapsulate as enum
        case 'v':
            verbosity++;
            break;
        case '?':
            printf("Unknown option: -%c.\n", optopt);
            showHelp(argv[0]);
            return 1;
        default:
            assert(false /* This is never supposed to be reached */);
        }
    }

    if (optind + 1 != argc) {
        printf("Unexpected number of positional arguments.\n");
        showHelp(argv[0]);
        return 1;
    }

    ifile = fopen(argv[optind], "rb");
    if (ifile == NULL) {
        ERR("Couldn't open %s to read", argv[optind]);
        return 2;
    }
    optind++;

    // TODO

    fclose(ifile);

    return 0;
}


void showBanner() {
    printf("######################################\n");
    printf("#                                    #\n");
    printf("#  Abel Executable Format Processor  #\n");
    printf("#           (c) Abel, 2020           #\n");
    printf("#                                    #\n");
    printf("######################################\n\n");

    printf("This program executes an .aef program.\n\n");
}

void showHelp(char *binName) {
    printf("Usage: %s ifile [-h] [-v]\n", binName);
    printf("  ifile      The input .aef program name\n");
    printf("  -h         Show this help message and exit\n");
    printf("  -v         Increase verbosity level\n\n");
}

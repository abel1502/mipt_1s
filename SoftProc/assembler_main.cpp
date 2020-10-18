#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <getopt.h>

#define CHECKSUM_NOIMPL

#include "constants.h"
#include "aef_file.h"
#include "assembler.h"
#include "general.h"


void showBanner();

void showHelp(char *binName);



int main(int argc, char **argv) {
    showBanner();

    FILE *ifile = NULL;
    FILE *ofile = NULL;
    extern int verbosity;

    int c = 0;

    while ((c = getopt(argc, argv, "+i:o:vh")) != -1) {
        switch (c) {
        case 'h':
            showHelp(argv[0]);
            return 1;  // TODO: Encapsulate as enum
        case 'i':
            ifile = fopen(optarg, "r");

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
            printf("Unknown option: -%c.\n", optopt);
            showHelp(argv[0]);
            return 1;  // TODO?: set opterr = 0 and produce error messages myself
        default:
            ERR("Shouldn't be reachable");
            abort();
        }
    }

    if (ifile == NULL || ofile == NULL) {
        printf("You must specify ifile and ofile to use this program.\n");
        showHelp(argv[0]);
        return 1;
    }

    if (optind != argc) {
        fclose(ifile);
        fclose(ofile);

        printf("Unexpected number of positional arguments.\n");
        showHelp(argv[0]);
        return 1;
    }


    code_t code = {};
    if (code_init(&code, verbosity >= 2) == NULL) {
        fclose(ifile);
        fclose(ofile);

        printf("Failed to initialize code buffer\n");
        return 3;
    }

    printf("Assembling the input file...\n");

    if (code_assembleFile(&code, ifile)) {
        code_free(&code);
        fclose(ifile);
        fclose(ofile);

        printf("Failed to assemble the input file.\n");
        return 3;
    }

    printf("Done, writing to the output file...\n");

    if (code_compileToFile(&code, ofile)) {
        code_free(&code);
        fclose(ifile);
        fclose(ofile);

        printf("Failed to write to the output file.\n");
        return 3;
    }

    printf("Done.\n");

    code_free(&code);
    fclose(ifile);
    fclose(ofile);

    /*code_t testCode = {};
    code_init(&testCode, true);

    FILE *ifile = fopen("test.aaf", "r");
    assert(!code_assembleFile(&testCode, ifile));
    fclose(ifile);

    FILE *ofile = fopen("test.aef", "wb");
    assert(!code_compileToFile(&testCode, ofile));
    fclose(ofile);

    code_free(&testCode);*/

    return 0;
}


void showBanner() {
    printf("####################################\n");
    printf("#                                  #\n");
    printf("#  Abel Assembly Format Assembler  #\n");
    printf("#          (c) Abel, 2020          #\n");
    printf("#                                  #\n");
    printf("####################################\n\n");

    printf("This program assembles an .aaf file into an .aef program.\n\n");
}

void showHelp(char *binName) {
    printf("Usage: %s [-h] -i ifile -o ofile [-v]\n", binName);
    printf("  -h         Show this help message and exit\n");
    printf("  -i ifile   The input .aaf file name\n");
    printf("  -o ofile   The output .aef file name\n");
    printf("  -v         Increase verbosity level\n\n");
}


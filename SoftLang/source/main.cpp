#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <getopt.h>

#include "general.h"
#include "parser.h"
#include "filebuf.h"


using namespace SoftLang;


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
    printf("Usage:  %s [-h] [-v] [-r] -i ifile -o ofile\n"
           "  -h        - show this and exit\n"
           "  -v        - increase verbosity\n"
           "  -r        - reconstruct the same code into ofile instead of compilation\n"
           "  -i ifile  - name of the input file\n"
           "  -o ofile  - name of the output file\n"
           "\n", binName);
}


int main(int argc, char **argv) {
    FileBuf ibuf{};
    FILE *ofile = nullptr;
    Parser parser{};
    Program prog{};
    bool doReconstruct = false;

    #define CLEANUP_            \
        ibuf.dtor();            \
        parser.dtor();          \
        prog.dtor();            \
        if (ofile) {            \
            fclose(ofile);      \
            ofile = nullptr;    \
        }

    showBanner();

    int c = 0;

    while ((c = getopt(argc, argv, "+i:o:vhr")) != -1) {
        switch (c) {
        case 'h':
            CLEANUP_

            showHelp(argv[0]);
            return 0;
        case 'i':
            if (ibuf.ctor(optarg, "r")) {
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
        case 'r':
            doReconstruct = true;
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

    if (!ibuf.isInited() || !ofile) {
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

    REQUIRE(!parser.ctor(&ibuf));

    switch (parser.parse(&prog)) {
    case Parser::ERR_PARSER_OK:
        printf("Program parsed successfully\n");
        break;

    case Parser::ERR_PARSER_LEX:
        ERR("Syntax error (lexical)\n");
        return 3;

    case Parser::ERR_PARSER_SYNTAX:
        ERR("Syntax error\n");
        return 3;

    case Parser::ERR_PARSER_SYS:
        ERR("System error\n");
        return 2;

    default:
        assert(false);
        return 2;
    }

    if (doReconstruct) {
        prog.reconstruct(ofile);
    } else {
        if (prog.compile(ofile)) {
            ERR("Failed to compile the program.\n"
                "(If there aren't any more precise error messages above,\n"
                " this might be due to a system error. In that case you\n"
                " should try to rerun the program, check that the output\n"
                "file is writable and if that doesn't help, report the\n"
                "problem at github.com/abel1502/mipt_1s .");

            return 3;
        }
    }

    printf("Done.\n\n");

    CLEANUP_

    return 0;

    #undef CLEANUP_
}

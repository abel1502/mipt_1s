#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <getopt.h>
#include <cstring>
#include <cctype>

#include "tree.h"
#include "general.h"


using namespace SomethingTree;  // I may do this, right?


static void showHelp(const char *binName) {
    printf("Usage:  %s [-h] [-v] [-i ifile] [-o ofile]\n"
           "  -h        - show this and exit\n"
           "  -v        - increase verbosity\n"
           "  -i ifile  - the name of the input .dtf database file\n"
           "              (if not specified, the decision tree will initially be blank)\n"
           "  -o ofile  - the name of the output .dtf database file\n"
           "              (if not specified, result won't be saved).\n"
           "\n", binName);
}


int main(int argc, char **argv) {
    FILE *ifile = NULL;
    FILE *ofile = NULL;

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


    DecisionTree dt{};
    dt.ctor();

    if (ifile) {
        dt.deserialize(ifile);

        fclose(ifile);
    }

    char tmpCompare[81] = "";

    while (true) {
        printf(" > ");

        char cmd[81] = "";
        size_t cmdLen = 0;

        fgets(cmd, 80, stdin);
        cmdLen = strlen(cmd);
        if (cmdLen > 0)  cmd[--cmdLen] = '\0';

        #define CMD_CASE_(CMD, CODE) \
            if (strncmp(CMD, cmd, sizeof(CMD) - 1) == 0 && (isspace(cmd[sizeof(CMD) - 1]) || cmd[sizeof(CMD) - 1] == '\0')) { \
                cmdLen = sizeof(CMD); \
                CODE \
            } else

        CMD_CASE_("", )
        CMD_CASE_("save",
            FILE *sfile = fopen(cmd + cmdLen, "wb");

            printf("Saving to %s...\n", cmd + cmdLen);

            if (!sfile) {
                ERR("Failed to save.");
                continue;
            }

            printf("Done.\n");

            dt.serialize(sfile);

            fclose(sfile);
        )
        CMD_CASE_("dump",
            dt.dump();
        )
        CMD_CASE_("lookup",
            dt.lookup();
        )
        CMD_CASE_("define",
            dt.define(cmd + cmdLen);
        )
        CMD_CASE_("compare",
            strcpy(tmpCompare, cmd + cmdLen);
        )
        CMD_CASE_("against",
            dt.compare(tmpCompare, cmd + cmdLen);
        )
        CMD_CASE_("help",
            printf("Commands:\n"
                   "  help              - show this help\n"
                   "  exit              - pretty obvious, isn't it?)\n"
                   "  save <file>       - saves the current tree to 'file'\n"
                   "  dump              - dumps the current tree\n"
                   "  lookup            - initiate a lookup-add dialogue\n"
                   "  define <term>     - print a definition for 'term'\n"
                   "  compare <term1>   - compares term1...\n"
                   "  against <term2>   - ... agains term2 (have to be used in conjuction)\n"
                   "\n");
        )
        CMD_CASE_("exit",
            break;
        ) {
            ERR("Unknown command.");
            continue;
        }

        #undef CMD_CASE_
    }


    if (ofile) {
        dt.serialize(ofile);

        fclose(ofile);
    }

    dt.dtor();

    return 0;
}

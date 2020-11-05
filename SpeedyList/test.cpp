#include "../libs/test.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>


// ==== [ Stack inclusion ] ====
typedef double list_elem_t;
#define LIST_ELEM_FMT "%lg"
#define LIST_VALIDATION_LEVEL 3
#define LIST_DUMPNAME list_dump_test
#include "list.h"
// =============================

const int CMD_MAX_LEN = 100;
const char CMD_FMT[] = "%100s";

//--------------------------------------------------------------------------------

int main() {
    TEST_MAIN(
        ,
        test_list(765.01, 433.02, 243.03, 678.04);
        TEST_SMSG("Passed All.");
        ,
    )

    printf("\n ===== [ Welcome to the interactive testing shell for SpeedyList. ] ===== \n");

    char cmd[CMD_MAX_LEN + 1] = "";

    list_t *lst = list_new(16);
    assert(lst != NULL);

    while (true) {
        printf(" > ");

        if (scanf(CMD_FMT, cmd) != 1) {
            printf("Failed to read cmd.\n");
            printf("(%s)\n", cmd);
            break;
        }

        #define CMD_CASE_(NAME, CODE) \
            if (strcmp(cmd, NAME) == 0) { \
                CODE \
            } else

        #define CMD_GETARG_(FMT, ARG) \
            if (scanf(FMT, &ARG) != 1) { \
                printf("Expected an argument of type \"%s\"\n", FMT); \
                continue; \
            }

        CMD_CASE_("insertAft",
            int arg = -1;
            CMD_GETARG_("%d", arg);

            double val = -1;
            CMD_GETARG_("%lg", val);

            if (list_insertAfter(lst, arg, val)) {
                printf("Failed.\n");
                continue;
            }
        )
        CMD_CASE_("insertBef",
            int arg = -1;
            CMD_GETARG_("%d", arg);

            double val = -1;
            CMD_GETARG_("%lg", val);

            if (list_insertBefore(lst, arg, val)) {
                printf("Failed.\n");
                continue;
            }
        )
        CMD_CASE_("pushFront",
            double val = -1;
            CMD_GETARG_("%lg", val);

            if (list_pushFront(lst, val)) {
                printf("Failed.\n");
                continue;
            }
        )
        CMD_CASE_("pushBack",
            double val = -1;
            CMD_GETARG_("%lg", val);

            if (list_pushBack(lst, val)) {
                printf("Failed.\n");
                continue;
            }
        )
        CMD_CASE_("remove",
            int arg = -1;
            CMD_GETARG_("%d", arg);

            double value = -1;

            if (list_remove(lst, arg, &value)) {
                printf("Failed.\n");
                continue;
            }

            printf("%lg\n", value);
        )
        CMD_CASE_("popFront",
            double value = -1;

            if (list_popFront(lst, &value)) {
                printf("Failed.\n");
                continue;
            }

            printf("%lg\n", value);
        )
        CMD_CASE_("popBack",
            double value = -1;

            if (list_popBack(lst, &value)) {
                printf("Failed.\n");
                continue;
            }

            printf("%lg\n", value);
        )
        CMD_CASE_("findByInd",
            int arg = -1;
            CMD_GETARG_("%d", arg);

            double value = 0;

            if (list_findByIndex(lst, arg, &value)) {
                printf("Not found.\n");
                continue;
            }

            printf("%lg\n", value);
        )
        CMD_CASE_("getVal",
            int arg = -1;
            CMD_GETARG_("%d", arg);

            list_node_t *node = list_getNode(lst, arg);

            if (node == NULL) {
                printf("Out of range.\n");
                continue;
            }

            printf("%d -> %d -> %d: (%lg)\n", node->prev, arg, node->next, node->value);
        )
        CMD_CASE_("enterArrMode",
            if (list_enterArrayMode(lst)) {
                printf("Failed.\n");
                continue;
            }
            continue;
        )
        CMD_CASE_("clear",
            list_clear(lst);
        )
        CMD_CASE_("getSize",
            printf("%d\n", lst->size);
        )
        CMD_CASE_("isEmpty",
            printf("%d\n", list_isEmpty(lst));
        )
        CMD_CASE_("dump",
            list_dump(lst);
        )
        CMD_CASE_("help",
            printf("Commands: \n"
                   "  insertAft <node> <val>\n"
                   "  insertBef <node> <val>\n"
                   "  pushFront <val>\n"
                   "  pushBack <val>\n"
                   "  remove <node>\n"
                   "  popFront\n"
                   "  popBack\n"
                   "  findByInd <ind>\n"
                   "  getVal <node>\n"
                   "  enterArrMode\n"
                   "  clear\n"
                   "  getSize\n"
                   "  isEmpty\n"
                   "  dump\n"
                   "  help\n"
                   "  exit\n"
                   "\n");
        )
        CMD_CASE_("exit",
            break;
        ) {
            printf("Unrecognized command. (Type 'exit' to exit or 'help' to see a list of avaivable commands)\n");
            continue;
        }

        #undef CMD_CASE_
    }

    list_destroy(lst);

    return EXIT_SUCCESS;
}


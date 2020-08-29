//#define TEST
#include "../libs/test.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>


#if !NDEBUG  // Okay, not gonna bother this time
#define ERR(msg, ...) do {fprintf(stderr, "[ERROR in %s()] " msg "\n", __func__, ##__VA_ARGS__);} while (0)
#else
#definne ERR(msg, ...) do {} while (0)
#endif

//--------------------------------------------------------------------------------

const int MAX_LINE = 80;
const int MAX_LINES = 2048;

//--------------------------------------------------------------------------------

void showBanner(void);
void showUsage(char * binname);
long int fsize(FILE * ifile);
int readLine(FILE * ifile, char * line, int maxLen);
int readLines(FILE * ifile, char ** lines, int maxLines);

//================================================================================

int main(int argc, char **argv) {
    TEST_MAIN(
        //
        ,
    )

    showBanner();

    if (argc != 2) {
        showUsage(argv[0]);
        ERR("Bad arg count");
        return EXIT_FAILURE;
    }

    FILE * ifile = fopen(argv[1], "r");
    char ** lines = (char **) malloc(sizeof(char *) * MAX_LINES);
    if (lines == NULL) {
        ERR("Can't allocate space for lines");
        return EXIT_FAILURE;
    }
    int lineCnt = readLines(ifile, lines, MAX_LINES);
    if (lineCnt < 0) {
        ERR("Error while reading lines");
        return EXIT_FAILURE;
    }
    printf("%d\n", lineCnt);

    for (int i = 0; i < lineCnt; ++i) {
        free(lines[i]);
    }
    free(lines);
    return EXIT_SUCCESS;
}

//================================================================================

void showBanner(void) {
    printf("################################\n"
           "#                              #\n"
           "#         Shakespeare          #\n"
           "#        (c) Abel, 2020        #\n"
           "#                              #\n"
           "################################\n");
    printf("This program sorts a Shakespearean poem in several funky ways.\n"
           "I'm too lazy to explain. Just give it a try).\n\n");
}

void showUsage(char * binname) {
    printf("Usage: %s ifile\n\n"
           "ifile - The file containing the source poem\n"
           "(The results are placed in files with the same names with extra prefixes)\n\n", binname);
}


long int fsize(FILE * ifile) {
    fseek(ifile, 0, SEEK_END);
    long int isize = ftell(ifile);
    fseek(ifile, 0, SEEK_SET);
    return isize;
}


int readLine(FILE * ifile, char * line, int maxLen) {
    int cur = fgetc(ifile);
    for (int pos = 0; cur != EOF && cur != '\n' && pos < maxLen; ++pos) {
        line[pos] = cur;
        cur = fgetc(ifile);
    }
    if (cur == '\n') {
        return 0;
    } else if (cur == EOF) {
        return 1;
    } else {
        ERR("Line too long");
        return 2;
    }
}

int readLines(FILE * ifile, char ** lines, int maxLines) {
    int state = 0;
    int i = 0;
    for (; state == 0 && i < maxLines; ++i) {
        lines[i] = (char *) malloc(sizeof(char) * MAX_LINE);
        if (lines[i] == NULL) {
            ERR("Can't allocate space for a line");
            return -1;
        }
        state = readLine(ifile, lines[i], MAX_LINE);
    }
    if (state == 2) {
        ERR("Line #%d too long", i);
        return -2;
    }
    return i;
}

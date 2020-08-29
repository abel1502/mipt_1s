#define TEST
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

const int MAX_POEM_LINE = 128; // These values are intended for Hamlet specifically,
const int MAX_RAW_LINE = 1024; // but should probably work for most other poems as well

//--------------------------------------------------------------------------------

void showBanner(void);
void showUsage(char * binname);
long int fsize(FILE * ifile);
int readPoemLine(FILE * ifile, char * line, int maxLen);  //!
char ** readPoemLines(FILE * ifile);  //!

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
    long int isize = fsize(ifile);
    char * text = (char *) malloc(isize + 1);
    fread((void *) text, sizeof(char), isize, ifile);
    fclose(ifile);

    // Special lines: any word in caps, "["..."]", "Exit"

    long int linecnt;
    long int * linestarts;
}

//================================================================================

void showBanner(void) {
    printf("################################\n"
           "#                              #\n"
           "#         Shakespeare          #\n"
           "#        (c) Abel, 2020        #\n"
           "#                              #\n"
           "################################\n");
    printf("This program sorts a poem (preferably Shakespearean) in several funky ways.\n"
           "I'm too lazy to explain. Just try it).\n\n");
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


int readPoemLine(FILE * ifile, char * line, int maxLen) {
    /*
     There are several special lines, which don't rhyme and aren't even
     part of the poem. Here are those I was able to identify:
      - Empty lines
      - Anything with two or more characters in caps one after the other
      - Anything with a square bracket
      - Lines explicitly commented out with a "#" (In this case, the general header)
    */
    int cur = fgetc(ifile);
    int pos = 0;
    while (cur != EOF && cur != '\n' && pos < maxLen) {
        line[pos] = cur;
        pos++;
        cur = fgetc(ifile);
    }
}

char ** readPoemLines(FILE * ifile) {

}

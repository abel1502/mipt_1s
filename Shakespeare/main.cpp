//#include <TXLib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>


#if !NDEBUG  // Okay, not gonna bother this time
#define ERR(msg, ...) do {fprintf(stderr, "[ERROR in %s()] " msg "\n", __func__, ##__VA_ARGS__);} while (0)
#else
#definne ERR(msg, ...) do {} while (0)
#endif


typedef struct lines {
    //
} lines_t;


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


int main(int argc, char **argv) {
    showBanner();

    if (argc != 2) {
        showUsage(argv[0]);
        ERR("Bad arg count");
        return EXIT_FAILURE;
    }

    FILE * ifile = fopen(argv[1], "r");
    isize = fsize(ifile);
    char * text = (char *) malloc(isize + 1);
    fread((void *) text, sizeof(char), isize, ifile);
    fclose(ifile);

    // Special lines: any word in caps, "["..."]", "Exit"

    long int linecnt;
    long int * linestarts;
}

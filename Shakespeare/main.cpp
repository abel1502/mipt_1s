/*
 Global TODO:

 # Add reverse sort and restoration
 # Wrap all error codes in enums
 - Move tests into files (and add new ones)
 # Move test code into a different build configuration
 # Open files in binary mode
 # Add digits to whitelist in the comparator
 # Store line's length in line_t
 / Remove struct names (and possibly remove _t from typedef ones)
 # Add ¸ and ¨
 # Add error handling to readLine
 - Add line_t 'methods'
 > Custom qsort
*/

#if __STDC_VERSION__ < 199901L && !defined(__cplusplus)
// This program will work with c99 and higher or presumably any c++
#error This program should only be compiled by to the C99 standards
#endif

#include <stdio.h>
#include <string.h>
#include <locale.h>

#include "general.h"
//#include "line.h"
#include "poem.h"


// It's a letter[] constant, but even when I moved the typedef to the top, it still didn't work
// (error: structured binding declaration cannot have type 'const letter' {aka 'const unsigned char'}|)
#define SEP "\n--------------------------------------------------------------------------------\n\n"


/**
 * Shows a constant banner at the beginning of execution
 */
void showBanner(void);

/**
 * Shows help on how to use the program
 */
void showUsage(const char *binname);

//================================================================================

int main(const int argc, const char **argv) {
    setlocale(LC_CTYPE, "Russian");



    showBanner();

    if (argc != 2) {
        showUsage(argv[0]);
        ERR("Bad arg count");
        return EXIT_FAILURE;
    }

    FILE * ifile = fopen(argv[1], "rb");
    if (ifile == NULL) {
        ERR("Couldn't open \'%s\' to read", argv[1]);
        return EXIT_FAILURE;
    }
    lines_t lines;  // Lines intentionally not init-ed, because readLines does it itself
    int result = readLines(ifile, &lines);
    fclose(ifile);
    if (result != SS_OK) {
        ERR("Error while reading lines");
        freeLines(&lines);
        return EXIT_FAILURE;
    }

    char newName[32] = "sorted_";
    if (strlen(newName) + strlen(argv[1]) > sizeof(newName)) {
        ERR("Binary\'s name is too long");
        freeLines(&lines);
        return EXIT_FAILURE;
    }
    strcat(newName, argv[1]);


    FILE * ofile = fopen(newName, "wb");
    if (ofile == NULL) {
        ERR("Couldn't open \'%s\' to write", newName);
        freeLines(&lines);
        return EXIT_FAILURE;
    }

    printf("Read %d lines, sorting...\n", lines.len);
    sortLines(&lines, cmpLines);

    printf("Done sorting, writing to %s\n", newName);
    if (writeLines(ofile, &lines) != SS_OK) {
        ERR("Error while writing lines");
        fclose(ofile);
        freeLines(&lines);
        return EXIT_FAILURE;
    }

    printf("Sorting again, this time in reverse...\n");
    sortLines(&lines, cmpLinesReverse);

    printf("Done sorting for the second time, writing to the same file\n");
    if (fprintf(ofile, SEP) < 0 || writeLines(ofile, &lines) != SS_OK) {
        ERR("Error while writing lines");
        fclose(ofile);
        freeLines(&lines);
        return EXIT_FAILURE;
    }

    printf("Okay, returning the original text now...\n");
    if (fprintf(ofile, SEP) < 0 || writeOriginalLines(ofile, &lines) != SS_OK) {
        ERR("Error while writing lines");
        fclose(ofile);
        freeLines(&lines);
        return EXIT_FAILURE;
    }

    printf("Done.\n");
    fclose(ofile);
    freeLines(&lines);
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
           "I\'m too lazy to explain. Just give it a try).\n"
           "\nEDIT: Okay, apparently Hamlet isn\'t a real poem, so I hereby\n"
           "proclaim that this also works on Eugene Onegin\n\n");
}

void showUsage(const char *binname) {
    printf("Usage: %s ifile\n\n"
           "ifile - The file containing the source poem\n"
           "(The results are placed in files with the same names with extra prefixes)\n\n", binname);
}

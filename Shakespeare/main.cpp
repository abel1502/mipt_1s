/*
 Global TODO:

 # Add reverse sort and restoration
 # Wrap all error codes in enums
 - Move tests into files (and add new ones)
 # Move test code into a different build configuration
 # Open files in binary mode
 # Add digits to whitelist in the comparator
 # Store line's length in line_t
 # Remove struct names (and possibly remove _t from typedef ones)
 # Add ¸ and ¨
 # Add error handling to readLine
 - Add line_t 'methods'
 ? Rename lines to poem
 > Custom qsort
 # Encapsulate comparators
 ? Create a getopt
 # Remove null checks before free
 # Remove analyzeFile
 - HTML log file
 # Error codes returned from main
 ? Organize main's return codes
 / Encapsulate the cleanup in main into a goto
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

//--------------------------------------------------------------------------------

static const char SEP[] = "\n--------------------------------------------------------------------------------\n\n";
static const size_t MAX_NAME = 80;
static const char SORTED_SUFFIX[] = ".sorted";

typedef enum {
    SSM_OK,
    SSM_ARGS,
    SSM_IO,
    SSM_READLINES,
    SSM_LONGNAME
} SHAKESPEARE_MAIN_ERROR;

//--------------------------------------------------------------------------------

/**
 * Shows a constant banner at the beginning of execution
 */
static void showBanner(void) {
    printf("################################\n" \
           "#                              #\n" \
           "#         Shakespeare          #\n" \
           "#        (c) Abel, 2020        #\n" \
           "#                              #\n" \
           "################################\n");
    printf("This program sorts a Shakespearean poem in several funky ways.\n"  \
           "I\'m too lazy to explain. Just give it a try).\n"                  \
           "\nEDIT: Okay, apparently Hamlet isn\'t a real poem, so I hereby\n" \
           "proclaim that this also works on Eugene Onegin\n\n");
}

/**
 * Shows help on how to use the program
 */
static void showUsage(const char *binname) {
    printf("Usage: %s ifile\n\n"                           \
           "ifile - The file containing the source poem (Encoded in WINDOWS-1251)\n" \
           "(The results are placed in files with the same names with extra prefixes)\n\n", binname);
}

//================================================================================

int main(const int argc, const char **argv) {
    setlocale(LC_CTYPE, "Russian");

    showBanner();

    if (argc != 2) {
        showUsage(argv[0]);
        ERR("Bad arg count");
        return SSM_ARGS;
    }

    FILE *ifile = fopen(argv[1], "rb");

    if (ifile == NULL) {
        ERR("Couldn't open \'%s\' to read", argv[1]);
        return SSM_IO;
    }

    lines_t lines = {};
    int result = readLines(ifile, &lines);

    fclose(ifile);

    if (result != SS_OK) {
        ERR("Error while reading lines");
        freeLines(&lines);
        return SSM_READLINES;
    }

    char newName[MAX_NAME] = "\0";
    size_t suffLen = strlen(SORTED_SUFFIX);
    strncpy(newName, argv[1], MAX_NAME - suffLen - 1);
    strncat(newName, SORTED_SUFFIX, suffLen);

    if (newName[MAX_NAME - 1] != '\0') {
        ERR("File name too long");
        freeLines(&lines);
        return SSM_LONGNAME;
    }

    FILE *ofile = fopen(newName, "wb");

    if (ofile == NULL) {
        ERR("Couldn't open \'%s\' to write", newName);
        freeLines(&lines);
        return SSM_IO;
    }

    printf("Read %d lines, sorting...\n", lines.len);
    sortLines(&lines, cmpLinesForward);

    printf("Done sorting, writing to %s\n", newName);
    if (writeLines(ofile, &lines) != SS_OK) {
        ERR("Error while writing lines");
        fclose(ofile);
        freeLines(&lines);
        return SSM_IO;
    }

    printf("Sorting again, this time in reverse...\n");
    sortLines(&lines, cmpLinesReverse);

    printf("Done sorting for the second time, writing to the same file\n");
    if (fprintf(ofile, SEP) < 0 || writeLines(ofile, &lines) != SS_OK) {
        ERR("Error while writing lines");
        fclose(ofile);
        freeLines(&lines);
        return SSM_IO;
    }

    printf("Okay, returning the original text now...\n");
    if (fprintf(ofile, SEP) < 0 || writeOriginalLines(ofile, &lines) != SS_OK) {
        ERR("Error while writing lines");
        fclose(ofile);
        freeLines(&lines);
        return SSM_IO;
    }

    printf("Done.\n");
    fclose(ofile);
    freeLines(&lines);
    return SSM_OK;
}


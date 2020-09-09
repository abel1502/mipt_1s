//#define TEST
#include "../libs/test.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <locale.h>
#include <ctype.h>


#if !NDEBUG  // Okay, not gonna bother this time
#define ERR(msg, ...) do {fprintf(stderr, "[ERROR in %s()] " msg "\n", __func__, ##__VA_ARGS__);} while (0)
#else
#definne ERR(msg, ...) do {} while (0)
#endif

//--------------------------------------------------------------------------------

const int MAX_LINE = 80;
const int MAX_LINES = 0x4000;

//--------------------------------------------------------------------------------

typedef unsigned char letter;

typedef struct lines {  // Gonna be a heap
    int len;
    letter ** vals;
} lines_t;

void showBanner(void);
void showUsage(const char * binname);
int readLine(FILE * ifile, letter * line, int maxLen);
int readLines(FILE * ifile, lines_t * lines, int maxLines);
int writeLines(FILE * ofile, lines_t * lines);
int initLines(lines_t * lines);
void sortLines(lines_t * lines);
bool isLetter(letter c);
int cmpLines(const void * first, const void * second);  // Attention: compares two strings, not lines_t!
void freeLines(lines_t * lines);

//================================================================================

int main(const int argc, const char **argv) {
    setlocale(LC_ALL, "Russian");

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
    lines_t lines;
    initLines(&lines);
    int result = readLines(ifile, &lines, MAX_LINES);
    fclose(ifile);
    if (result) {
        ERR("Error while reading lines");
        return EXIT_FAILURE;
    }

    //printf("%d\n", lines.len);
    sortLines(&lines);

    char newName[32] = "sorted_";
    if (strlen(newName) + strlen(argv[1]) > sizeof(newName)) {
        ERR("Binary\'s name is too long");
        return EXIT_FAILURE;
    }
    strcat(newName, argv[1]);
    FILE * ofile = fopen(newName, "w");
    result = writeLines(ofile, &lines);
    fclose(ofile);
    if (result) {
        ERR("Error while writing lines");
        return EXIT_FAILURE;
    }

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

void showUsage(const char * binname) {
    printf("Usage: %s ifile\n\n"
           "ifile - The file containing the source poem\n"
           "(The results are placed in files with the same names with extra prefixes)\n\n", binname);
}

int readLine(FILE * ifile, letter * line, int maxLen) {
    int cur = fgetc(ifile);
    int pos = 0;
    for (; cur != EOF && cur != '\n' && pos < maxLen; ++pos) {
        line[pos] = cur;
        cur = fgetc(ifile);
    }
    line[pos] = '\0';
    if (cur == '\n') {
        return 0;
    } else if (cur == EOF) {
        return 1;
    } else {
        ERR("Line too long");
        return 2;
    }
}

int readLines(FILE * ifile, lines_t * lines, int maxLines) {
    int state = 0;
    int i = 0;
    for (; state == 0 && i < maxLines; ++i) {
        lines->vals[i] = (letter *) malloc(sizeof(letter) * MAX_LINE);
        if (lines->vals[i] == NULL) {
            ERR("Can\'t allocate space for a line");
            return 1;
        }
        state = readLine(ifile, lines->vals[i], MAX_LINE - 1);
        lines->len++;
    }
    if (state == 1) { // Empty last line
        free(lines->vals[lines->len - 1]);
        lines->len--;
    }
    if (state == 2) {
        ERR("Line #%d too long", i);
        return 2;
    }
    return 0;
}

int writeLines(FILE * ofile, lines_t * lines) {
    for (int i = 0; i < lines->len; ++i) {
        if (fputs((const char *)lines->vals[i], ofile) == EOF) {
            ERR("Can\'t write line #%d", i);
            return 1;
        }
        fputc('\n', ofile);
    }
    return 0;
}

int initLines(lines_t * lines) {
    lines->vals = (letter **) calloc(MAX_LINES, sizeof(letter *));
    if (lines->vals == NULL) {
        ERR("Can't allocate space for lines");
        return 1;
    }
    lines->len = 0;
    return 0;
}

void sortLines(lines_t * lines) {
    qsort(lines->vals, lines->len, sizeof(lines->vals[0]), cmpLines);
}

bool isLetter(letter c) {
    return isalpha(c) || ((letter)'à' <= c && c <= (letter)'ÿ') || ((letter)'À' <= c && c <= (letter)'ß');
}

int cmpLines(const void * first, const void * second) {
    letter * first_s = *(letter **)first;
    letter * second_s = *(letter **)second;

    int i = 0, j = 0;
    while (i < MAX_LINE && j < MAX_LINE && first_s[i] && second_s[j]) {
        if (isLetter(first_s[i]) && isLetter(second_s[j])) {
            int res = (int)first_s[i] - (int)second_s[j];
            if (res) return res;
            i++; j++;
            continue;
        }
        if (!isLetter(first_s [i])) i++;
        if (!isLetter(second_s[j])) j++;
    }
    return 0;
}

void freeLines(lines_t * lines) {
    for (int i = 0; i < lines->len; ++i) {
        free(lines->vals[i]);
    }
    free(lines->vals);
    lines->len = 0;
}


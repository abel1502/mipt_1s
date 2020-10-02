#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "line.h"

static int cmpLines_(const line_t *a, const line_t *b, const int step);

//--------------------------------------------------------------------------------

SS_ERROR readLine(line_t *line, letter *text, size_t *offset) {
    assert(line != NULL);
    assert(text != NULL);
    assert(offset != NULL);

    line->val = text + (*offset);

    for (line->len = 0;
         text[*offset] != '\0' && line->len < MAX_LINE;
         (*offset)++, line->len++) {}

    if (text[*offset] != '\0') {
        ERR("Line too long");
        return SS_TOOBIG;
    }

    line->len++;
    (*offset)++;
    return SS_OK;
}

//--------------------------------------------------------------------------------

int isRelevant(letter c) {
    return isalnum(c);
}

//--------------------------------------------------------------------------------

inline int inBounds(const line_t *line, int offset) {
    return 0 <= offset && offset < line->len;
}

//--------------------------------------------------------------------------------

static int nextLetter(const line_t *line, int *offset, int step) {
    while (inBounds(line, *offset) && !isRelevant(line->val[*offset])) {
        (*offset) += step;
    }
    return !inBounds(line, *offset);
}

//--------------------------------------------------------------------------------

static int cmpLines_(const line_t *a, const line_t *b, const int step) {
    assert(a != NULL);
    assert(b != NULL);
    assert(step != 0);

    int a_offset = 0;
    int b_offset = 0;
    if (step < 0) {
        a_offset = a->len - 1;
        b_offset = b->len - 1;
    }

    int a_noNext = 0;
    int b_noNext = 0;

    while (1) {
        a_noNext = nextLetter(a, &a_offset, step);
        b_noNext = nextLetter(b, &b_offset, step);

        if (a_noNext || b_noNext) {
            break;
        }

        int res = (int)(a->val[a_offset]) - (int)(b->val[b_offset]);
        if (res) return res;

        a_offset += step;
        b_offset += step;
    }

    if ( a_noNext && !b_noNext) return -1;
    if (!a_noNext &&  b_noNext) return 1;
    return 0;
}

//--------------------------------------------------------------------------------

int cmpLinesForward(const void *a, const void *b) {
    return cmpLines_((line_t *)a, (line_t *)b, 1);
}

//--------------------------------------------------------------------------------

int cmpLinesReverse(const void *a, const void *b) {
    return cmpLines_((line_t *)a, (line_t *)b, -1);
}

//--------------------------------------------------------------------------------

line_t *assignLiteralLine(line_t *line, char *value) {
    line->val = (letter *)value;
    line->len = (unsigned char)strlen(value) + 1;

    #if DEBUG_SSORT
    printf("%u [%p] %s\n", line->len, line->val, line->val);
    #endif // DEBUG_SSORT

    return line;
}


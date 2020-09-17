#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "line.h"

SS_ERROR readLine(letter *text, size_t *offset, line_t *line) {
    line->val = text + (*offset);
    for (line->len = 0; text[*offset] != '\n' && line->len < MAX_LINE; (*offset)++, line->len++) {}
    if (text[*offset] != '\n') {
        ERR("Line too long");
        return SS_TOOBIG;
    }
    text[*offset] = '\0';
    line->len++;
    (*offset)++;
    return SS_OK;
}

int isRelevant(letter c) {
    return isalnum(c);
}

int cmpLines(const void *a, const void *b) {
    assert(a != NULL);
    assert(b != NULL);
    const letter * a_str = (*(const line *)a).val;
    const letter * b_str = (*(const line *)b).val;
    const unsigned char a_len = (*(const line *)a).len;
    const unsigned char b_len = (*(const line *)b).len;

    int i = 0, j = 0;
    while (i < a_len && j < b_len) {
        if (isRelevant(a_str[i]) && isRelevant(b_str[j])) {
            int res = (int)a_str[i] - (int)b_str[j];
            if (res) return res;
            i++; j++;
            continue;
        }
        if (!isRelevant(a_str[i])) i++;
        if (!isRelevant(b_str[j])) j++;
    }
    while (i < a_len && !isRelevant(a_str[i])) i++;
    while (j < b_len && !isRelevant(b_str[j])) j++;
    if (i >= a_len && j < b_len) return -1;
    if (i < a_len && j >= b_len) return 1;
    return 0;
}

int cmpLinesReverse(const void *a, const void *b) {
    assert(a != NULL);
    assert(b != NULL);
    const letter * a_str = (*(const line *)a).val;
    const letter * b_str = (*(const line *)b).val;
    const unsigned char a_len = (*(const line *)a).len;
    const unsigned char b_len = (*(const line *)b).len;

    int i = a_len - 1, j = b_len - 1;
    while (i >= 0 && j >= 0) {
        if (isRelevant(a_str[i]) && isRelevant(b_str[j])) {
            int res = (int)a_str[i] - (int)b_str[j];
            if (res) return res;
            i--; j--;
            continue;
        }
        if (!isRelevant(a_str[i])) i--;
        if (!isRelevant(b_str[j])) j--;
    }
    while (i < 0 && !isRelevant(a_str[i])) i--;
    while (j < 0 && !isRelevant(b_str[j])) j--;
    if (i < 0 && j >= 0) return -1;
    if (i >= 0 && j < 0) return 1;
    return 0;
}

line_t * assignLiteralLine(line_t *line, char *value) {
    line->val = (letter *)value;
    line->len = (unsigned char)strlen(value) + 1;
    return line;
}

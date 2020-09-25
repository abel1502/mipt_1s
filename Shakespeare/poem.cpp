#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "poem.h"


SS_ERROR readLines(FILE *ifile, lines_t *lines) {
    assert(ifile != NULL);
    assert(lines != NULL);
    size_t textLen = 0;
    int lineCnt = 0;
    analyzeFile(ifile, &lineCnt, &textLen);
    if (textLen <= 2) {
        ERR("Empty file");
        return SS_TOOSMALL;
    }

    if (initLines(lines, lineCnt, textLen) != SS_OK) {
        ERR("Trouble initializing lines");
        return SS_NOALLOC;
    }

    if (fread(lines->text, sizeof(letter), textLen - 1, ifile) != textLen - 1) {
        ERR("Insufficient data read from file. Race condition, huh?");
        return SS_RACECOND;
    }
    lines->textLen = textLen;
    lines->text[textLen - 1] = '\0';
    if (lines->text[textLen - 2] != '\n') {
        lines->text[textLen - 1] = '\n';
        lineCnt++;
    } else {
        lines->textLen--;
        textLen--;
    }

    size_t offset = 0;
    for (lines->len = 0; lines->len < lineCnt; lines->len++) {
        if (readLine(lines->text, &offset, &lines->vals[lines->len]) != SS_OK) {
            ERR("Can't parse line #%d", lines->len);
            return SS_INNER;
        }
    }

    if (lines->len != lineCnt) {
        ERR("Wrong number of lines (%d instead of %d)", lines->len, lineCnt);
        return SS_RACECOND;
    }
    return SS_OK;
}

//--------------------------------------------------------------------------------

SS_ERROR writeLines(FILE *ofile, lines_t *lines) {
    assert(ofile != NULL);
    assert(lines != NULL);
    for (int i = 0; i < lines->len; ++i) {
        if (fputs((const char *)lines->vals[i].val, ofile) == EOF) {
            ERR("Can\'t write line #%d", i);
            return SS_INNER;
        }
        fputc('\n', ofile);
    }
    return SS_OK;
}

//--------------------------------------------------------------------------------

SS_ERROR writeOriginalLines(FILE *ofile, lines_t *lines) {
    for (size_t i = 0; i < lines->textLen; ++i) {
        letter cur = lines->text[i];
        if (cur == '\0') cur = '\n';
        if (fputc(cur, ofile) == EOF) {
            #pragma GCC diagnostic push
            // %z is supported starting from c99 (and the sufficiency of the current
            // compiler is ensured by a preprocessor check in the beginning of the file),
            // but the compiler for some reason wants to warn me that this specifier is unknown
            // and the argument count is wrong.
            #pragma GCC diagnostic ignored "-Wformat"
            #pragma GCC diagnostic ignored "-Wformat-extra-args"
            ERR("Can\'t write letter #%zu", i);
            #pragma GCC diagnostic pop
            return SS_INNER;
        }
    }
    return SS_OK;
}

//--------------------------------------------------------------------------------

SS_ERROR initLines(lines_t *lines, int maxLines, size_t maxLen) {
    assert(lines != NULL);
    lines->vals = (line_t *) calloc(maxLines, sizeof(line_t));
    if (lines->vals == NULL) {
        ERR("Can't allocate space for lines");
        return SS_NOALLOC;
    }
    lines->text = (letter *)malloc(maxLen * sizeof(letter));
    if (lines->text == NULL) {
        ERR("Can't allocate space for text");
        return SS_NOALLOC;
    }
    lines->len = 0;
    lines->textLen = 0;
    return SS_OK;
}

//--------------------------------------------------------------------------------

void sortLines(lines_t *lines, comparator_t cmp) {
    assert(lines != NULL);
    assert(cmp != NULL);

    qsort(lines->vals, lines->len, sizeof(lines->vals[0]), cmp);
}

//--------------------------------------------------------------------------------

void customSortLines(lines_t *lines, comparator_t cmp) {
    assert(lines != NULL);
    assert(cmp != NULL);

    ssort(lines->vals, 0, lines->len, sizeof(lines->vals[0]), cmp);
}

//--------------------------------------------------------------------------------

void freeLines(lines_t *lines) {
    if (lines == NULL) return;
    if (lines->vals != NULL) free(lines->vals);
    if (lines->text != NULL) free(lines->text);
    lines->vals = NULL;
    lines->text = NULL;
    lines->len = 0;
}

//--------------------------------------------------------------------------------

void analyzeFile(FILE *ifile, int *lineCnt, size_t *length) {
    assert(ifile != NULL);
    assert(lineCnt != NULL);
    assert(length != NULL);
    fseek(ifile, 0L, SEEK_SET);
    (*lineCnt) = 0;
    (*length) = 0;
    int cur = 0;
    do {
        cur = fgetc(ifile);
        (*length)++;
        if (cur == '\n') (*lineCnt)++;
    } while (cur != EOF);
    fseek(ifile, 0L, SEEK_SET);
//--------------------------------------------------------------------------------

static void _ssort_swap(void *base, size_t a, size_t b, size_t size) {
    assert(base != NULL);

    // TODO: Basic cases automation

    char *a_ptr = (char *)base + a * size;
    char *b_ptr = (char *)base + b * size;

    for (size_t i = 0; i < size; ++i) {
        char tmp = *a_ptr;
        *a_ptr = *b_ptr;
        *b_ptr = tmp;

        a_ptr++;
        b_ptr++;
    }
}

//--------------------------------------------------------------------------------

static size_t _ssort_partition(void *base, size_t low, size_t high, size_t size, comparator_t cmp) {
    assert(base != NULL);
    assert(low + 1 < high);

    #define GET_ITEM(i) (void *)((char *)base + (i) * size)

    void *pivot = GET_ITEM((low + high) / 2);
    size_t i = low;
    size_t j = high;

    while (1) {
        while (i < high && cmp(GET_ITEM(i), pivot) < 0) i++;
        while (j > low  && cmp(GET_ITEM(j), pivot) > 0) j--;
        if (i >= j) return j;

        _ssort_swap(base, i, j, size);
        i++;
        j--;
    }

    #undef GET_ITEM
}

//--------------------------------------------------------------------------------

void ssort(void* base, size_t low, size_t high, size_t size, comparator_t cmp) {
    assert(base != NULL);
    assert(cmp != NULL);
    assert(size > 0);

    if (low + 1 >= high) return;
    size_t separator = _ssort_partition(base, low, high, size, cmp);
    ssort(base, low, separator, size, cmp);
    ssort(base, separator + 1, high, size, cmp);
}

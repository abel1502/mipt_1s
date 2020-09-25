#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/stat.h>

#include "poem.h"


SS_ERROR readLines(FILE *ifile, lines_t *lines) {
    assert(ifile != NULL);
    assert(lines != NULL);

    (*lines) = {};

    if (getFSize(ifile, &(lines->textLen)) != SS_OK) {
        ERR("Couldn't retrieve file size");
        return SS_INNER;
    }
    if (lines->textLen <= 2) {
        ERR("Empty file");
        return SS_TOOSMALL;
    }

    lines->text = (letter *)calloc(lines->textLen, sizeof(letter));
    if (lines->text == NULL) {
        ERR("Can't allocate space for text");
        return SS_NOALLOC;
    }

    if (fread(lines->text, sizeof(letter), lines->textLen - 1, ifile) != lines->textLen - 1) {
        ERR("Insufficient data read from file");
        return SS_RACECOND;
    }

    lines->text[lines->textLen - 1] = '\0';
    if (lines->text[lines->textLen - 2] != '\n') {
        lines->text[lines->textLen - 1] = '\n';
    } else {
        lines->textLen--;
    }

    for (size_t i = 0; i < lines->textLen; ++i) {
        if (lines->text[i] == '\n') {
            lines->text[i] = '\0';
            lines->len++;
        }
    }

    lines->vals = (line_t *) calloc(lines->len, sizeof(line_t));
    if (lines->vals == NULL) {
        ERR("Can't allocate space for lines");
        return SS_NOALLOC;
    }

    size_t offset = 0;
    for (int lineInd = 0; lineInd < lines->len; ++lineInd) {
        if (readLine(&(lines->vals[lineInd]), lines->text, &offset) != SS_OK) {
            ERR("Can't parse line #%d", lineInd);
            return SS_INNER;
        }
    }
    assert(offset == lines->textLen);

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
    assert(ofile != NULL);
    assert(lines != NULL);

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

    lines->vals = (line_t *)calloc(maxLines, sizeof(line_t));
    if (lines->vals == NULL) {
        ERR("Can't allocate space for lines");
        return SS_NOALLOC;
    }

    lines->text = (letter *)calloc(maxLen, sizeof(letter));
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

    free(lines->vals);
    lines->vals = NULL;

    free(lines->text);
    lines->text = NULL;

    lines->len = 0;
}

//--------------------------------------------------------------------------------

SS_ERROR getFSize(FILE *ifile, size_t *length) {
    assert(ifile != NULL);
    assert(length != NULL);

    struct stat buf = {};

    if (fstat(ifile->_file, &buf)) {
        ERR("Couldn't retrieve file size");
        return SS_INNER;
    }
    (*length) = buf.st_size + 1;

    return SS_OK;
}

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

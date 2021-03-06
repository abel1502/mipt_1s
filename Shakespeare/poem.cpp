#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/stat.h>

#include "poem.h"

static size_t ssort_partition_(void *base, size_t low, size_t high, size_t size, comparator_t cmp);
static void ssort_swap_(void *base, size_t a, size_t b, size_t size);
static inline void *ssort_getItem_(void *base, size_t size, size_t i);

#if DEBUG_SSORT
static void ssort_logLine_(void *base, size_t i);
static void ssort_logLines_(void *base, size_t low, size_t high);
#endif // DEBUG_SSORT

//--------------------------------------------------------------------------------

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
            ERR("Can\'t write letter #%zu", i);
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

    ssort(lines->vals, 0, lines->len - 1, sizeof(lines->vals[0]), cmp);
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

#if DEBUG_SSORT

static void ssort_logLine_(void *base, size_t i) {
    line_t *arr = (line_t *)base;

    printf("%2zu) [%p] (%u) %s\n", i, arr[i].val, arr[i].len, arr[i].val);
}

//--------------------------------------------------------------------------------

static void ssort_logLines_(void *base, size_t low, size_t high) {
    printf("====== [%zu, %zu]\n", low, high);
    for (; low <= high; ++low) {
        ssort_logLine_(base, low);
    }
    printf("======\n");
}


#endif // DEBUG_SSORT

//--------------------------------------------------------------------------------

static inline void *ssort_getItem_(void *base, size_t size, size_t i) {
    return (void *)((char *)base + i * size);
}

//--------------------------------------------------------------------------------

static void ssort_swap_(void *base, size_t a, size_t b, size_t size) {
    assert(base != NULL);

    #define HANDLE_BASIC_CASE(expType) \
        if (size == sizeof(expType)) {           \
            expType *arr = (expType *)base;     \
            expType tmp = arr[a];               \
            arr[a] = arr[b];                    \
            arr[b] = tmp;                       \
            return;                             \
        }

    HANDLE_BASIC_CASE(char);
    HANDLE_BASIC_CASE(short);
    HANDLE_BASIC_CASE(long);
    HANDLE_BASIC_CASE(long long);

    #undef HANDLE_BASIC_CASE

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

static size_t ssort_partition_(void *base, size_t low, size_t high, size_t size, comparator_t cmp) {
    assert(base != NULL);
    assert(low + 1 < high);

    void *pivot = calloc(1, size);
    assert(pivot != NULL);

    memcpy(pivot, ssort_getItem_(base, size, (low + high) / 2), size);

    size_t i = low;
    size_t j = high;

    #if DEBUG_SSORT
    printf("pivot: ");
    ssort_logLine_(base, (line_t *)pivot - (line_t *)base);
    #endif // DEBUG_SSORT

    while (1) {
        #if DEBUG_SSORT
        #define LOG_I ssort_logLine_(base, i); printf(" * %d\n", cmp(ssort_getItem_(base, size, i), pivot));
        #define LOG_J ssort_logLine_(base, j); printf(" * %d\n", cmp(pivot, ssort_getItem_(base, size, j));
        #else
        #define LOG_I
        #define LOG_J
        #endif // DEBUG_SSORT


        while (i <= high && cmp(ssort_getItem_(base, size, i), pivot) < 0) {
            LOG_I
            i++;
        } LOG_I

        #if DEBUG_SSORT
        printf("--\n");
        #endif // DEBUG_SSORT

        while (j >= low && j != (size_t)-1 && cmp(pivot, ssort_getItem_(base, size, j)) < 0) {
            LOG_I
            j--;
        } LOG_I

        #undef LOG_I
        #undef LOG_J

        #if DEBUG_SSORT
        printf("<%zu %zu>\n", i, j);
        #endif // DEBUG_SSORT

        if (i >= j) break;

        ssort_swap_(base, i, j, size);
        i++;
        j--;
    }

    free(pivot);

    return j;
}

//--------------------------------------------------------------------------------

void ssort(void* base, size_t low, size_t high, size_t size, comparator_t cmp) {
    assert(base != NULL);
    assert(cmp != NULL);
    assert(size > 0);

    #if DEBUG_SSORT
    ssort_logLines_(base, low, high);
    #endif // DEBUG_SSORT

    if (low + 1 > high) {
        return;
    } else if (low + 1 == high) {
        if (cmp(ssort_getItem_(base, size, low), ssort_getItem_(base, size, high)) > 0) {
            ssort_swap_(base, low, high, size);
        }
        return;
    }

    size_t separator = ssort_partition_(base, low, high, size, cmp);

    #if DEBUG_SSORT

    printf("sep = %zu\n", separator);

    ssort_logLines_(base, low, high);

    #endif // DEBUG_SSORT

    ssort(base, low, separator, size, cmp);
    ssort(base, separator + 1, high, size, cmp);
}


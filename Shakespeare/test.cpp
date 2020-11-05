#include "../libs/test.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

#include "general.h"
#include "line.h"
#include "poem.h"


void test_sortLines(lines_t *lines);
void test_cmpLines();
void test_isRelevant();

int main() {
    setlocale(LC_CTYPE, "Russian");
    TEST_MAIN(
        lines_t test_lines;
        initLines(&test_lines, 40, 1000);
        ,
        test_sortLines(&test_lines);
        test_cmpLines();
        test_isRelevant();
        TEST_SMSG("Passed All.");
        ,
        freeLines(&test_lines);
    )
}

void test_sortLines(lines_t *lines) {
    const int cnt = 5;
    letter values[cnt][20] = {"Привет", "При, вот", "!При, ват", "[123]", "Bonjour"};
    const letter result[cnt][20] = {"[123]", "Bonjour", "!При, ват", "Привет", "При, вот"};

    for (int i = 0; i < cnt; ++i) {
        lines->vals[i].val = (letter *)calloc(20, sizeof(letter));
        TEST_ASSERT_M(lines->vals[i].val != NULL, "Not enough RAM for the test");
        assignLiteralLine(&(lines->vals[i]), (char *)values[i]);
        lines->len++;
    }

    customSortLines(lines, cmpLinesForward);

    TEST_ASSERT(lines->len == cnt);
    for (int i = 0; i < cnt; ++i) {
        TEST_ASSERT_M(strcmp((const char *)lines->vals[i].val, (const char *)result[i]) == 0, "Expected %s on %ith place, got %s instead", result[i], i, lines->vals[i].val);
    }
}

void test_cmpLines() {
    #pragma GCC diagnostic push
    // This uses constant string literals as char pointers.
    // None of my code modifies the values, and this approach
    // saves a lot of time and memory on malloc's and strcpy's
    #pragma GCC diagnostic ignored "-Wcast-qual"
    #pragma GCC diagnostic ignored "-Wwrite-strings"

    line_t lineA, lineB;
    #define TEST_CMPLINES(direction, _lineA, relation, _lineB) \
        MACROFUNC( \
        assignLiteralLine(&lineA, (_lineA)); \
        assignLiteralLine(&lineB, (_lineB)); \
        { \
            int result = cmpLines##direction(&lineA, &lineB); \
            TEST_ASSERT_M(result relation 0, "%s %s %s", #_lineA, (result < 0 ? "<" : result > 0 ? ">" : "=="), #_lineB); \
        } \
        )

    TEST_CMPLINES(Forward, "123", ==, "#12!3\"");
    TEST_CMPLINES(Forward, "Привет, Андрей", >, "Hello, Andrew");
    TEST_CMPLINES(Forward, "", ==, "");
    TEST_CMPLINES(Forward, "", <, "йцукен");
    TEST_CMPLINES(Forward, "", ==, ",./");

    #undef TEST_CMPLINES

    #pragma GCC diagnostic pop
}

void test_isRelevant() {
    TEST_ASSERT(isRelevant('r'));
    TEST_ASSERT(isRelevant('H'));
    TEST_ASSERT(isRelevant('Ъ'));
    TEST_ASSERT(isRelevant('ф'));
    TEST_ASSERT(isRelevant('я'));
    TEST_ASSERT(isRelevant('ё'));
    TEST_ASSERT(isRelevant('Ё'));
    TEST_ASSERT(isRelevant('7'));

    TEST_ASSERT(!isRelevant(','));
    TEST_ASSERT(!isRelevant('!'));
    TEST_ASSERT(!isRelevant('\n'));
    TEST_ASSERT(!isRelevant(' '));
    TEST_ASSERT(!isRelevant('\0'));
}


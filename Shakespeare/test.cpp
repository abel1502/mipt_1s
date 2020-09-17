#if __STDC_VERSION__ < 199901L && !defined(__cplusplus)
// This program will work with c99 and higher or presumably any c++
#error This program should only be compiled by to the C99 standards
#endif

#define TEST
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

int main() {
    setlocale(LC_CTYPE, "Russian");
    TEST_MAIN(
        lines_t test_lines;
        initLines(&test_lines, 40, 1000);
        ,
        test_sortLines(&test_lines);
        test_cmpLines();
        $g; TEST_MSG("Passed All."); $d;
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

    sortLines(lines, cmpLines);

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
    #define TEST_CMPLINES(_lineA, relation, _lineB) \
        MACROFUNC( \
        assignLiteralLine(&lineA, (_lineA)); \
        assignLiteralLine(&lineB, (_lineB)); \
        { \
            int result = cmpLines(&lineA, &lineB); \
            TEST_ASSERT_M(result relation 0, "%s %s %s", #_lineA, (result < 0 ? "<" : result > 0 ? ">" : "=="), #_lineB); \
        } \
        )

    TEST_ASSERT(isRelevant('H'));  // Ignore this)
    TEST_CMPLINES("123", ==, "#12!3\"");
    TEST_CMPLINES("Привет, Андрей", >, "Hello, Andrew");
    TEST_CMPLINES("", ==, "");
    TEST_CMPLINES("", <, "йцукен");
    TEST_CMPLINES("", ==, ",./");

    #undef TEST_CMPLINES

    #pragma GCC diagnostic pop
}

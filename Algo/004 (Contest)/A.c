/*

Вася написал на длинной полоске бумаги большое число и решил похвастаться своему старшему брату Пете этим достижением. Но только он вышел из комнаты, чтобы позвать брата, как его сестра Катя вбежала в комнату и разрезала полоску бумаги на несколько частей. В результате на каждой части оказалось одна или несколько идущих подряд цифр.

Теперь Вася не может вспомнить, какое именно число он написал. Только помнит, что оно было очень большое. Чтобы утешить младшего брата, Петя решил выяснить, какое максимальное число могло быть написано на полоске бумаги перед разрезанием. Помогите ему!

Входные данные
Входной файл содержит одну или более строк, каждая из которых содержит последовательность цифр. Количество строк во входном файле не превышает 100, каждая строка содержит от 1 до 100 цифр. Гарантируется, что хотя бы в одной строке первая цифра отлична от нуля.

Выходные данные
Выведите в выходной файл одну строку — максимальное число, которое могло быть написано на полоске перед разрезанием.

Примеры
входные данные
2
20
004
66
выходные данные
66220004
входные данные
3
выходные данные
3

*/

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <sys/stat.h>

// For debug
//#define assert(stmt) if (!(stmt)) {exit(__LINE__);}


typedef struct lines_s lines_t;

struct lines_s {
    unsigned long long count;
    char **index;
    unsigned long long bufSize;
    char *buf;
};


unsigned long long fsize(FILE *ifile);

void lines_read(lines_t *self, FILE *ifile);

void lines_write(lines_t *self, FILE *ofile);

void lines_free(lines_t *self);

int customStrCmp(const void *a, const void *b);


int main() {
    FILE *fin  = fopen("number.in",  "r");
    FILE *fout = fopen("number.out", "w");

    assert(fin  != NULL);
    assert(fout != NULL);


    lines_t lines = {};

    lines_read(&lines, fin);

    qsort(lines.index, lines.count, sizeof(lines.index[0]), customStrCmp);

    lines_write(&lines, fout);

    lines_free(&lines);

    fclose(fin);
    fclose(fout);

    return 0;
}


unsigned long long fsize(FILE *ifile) {
    assert(ifile != NULL);

    struct stat buf = {};
    int res = fstat(ifile->_file, &buf);
    assert(res == 0);

    return buf.st_size + 1;

    /*fseek(ifile, 0, SEEK_END);
    unsigned long long size = ftell(ifile);
    fseek(ifile, 0, SEEK_SET);

    return size + 1;*/
}

void lines_read(lines_t *self, FILE *ifile) {
    assert(self != NULL);
    assert(ifile != NULL);

    self->bufSize = fsize(ifile);

    assert(self->bufSize >= 2);

    self->buf = (char *)calloc(self->bufSize, sizeof(char));
    assert(self->buf != NULL);

    // Oh my god... They have CRLFs in their input...
    self->bufSize = fread(self->buf, sizeof(char), self->bufSize - 1, ifile) + 1;
    //unsigned long long res = fread(self->buf, sizeof(char), self->bufSize - 1, ifile);
    //assert(res == self->bufSize - 1);

    self->buf[self->bufSize - 1] = '\0';
    if (self->buf[self->bufSize - 2] != '\n') {
        self->buf[self->bufSize - 1] = '\n';
    } else {
        self->bufSize--;
    }

    for (unsigned long long i = 0; i < self->bufSize; ++i) {
        if (self->buf[i] == '\n') {
            self->buf[i] = '\0';
            self->count++;
        }
    }

    self->index = (char **)calloc(self->count, sizeof(self->index[0]));
    assert(self->index != NULL);

    unsigned long long offset = 0;
    unsigned long long lineInd = 0;
    self->index[lineInd++] = self->buf + offset;

    for (; lineInd < self->count; ++offset) {
        if (self->buf[offset] == '\0') {
            self->index[lineInd++] = self->buf + offset + 1;
        }
    }
}

void lines_write(lines_t *self, FILE *ofile) {
    for (unsigned long long i = 0; i < self->count; ++i) {
        int res = fputs(self->index[i], ofile);
        assert(res != EOF);
    }
}

void lines_free(lines_t *self) {
    assert(self != NULL);

    free(self->buf);
    free(self->index);
}

int customStrCmp(const void *a, const void *b) {
    // Essentially, this compares a+b and b+a, but in a perverted way with no extra memory

    assert(a != NULL);
    assert(b != NULL);

    char *aStr = *(char **)a;
    char *bStr = *(char **)b;

    unsigned long long aInd = 0, bInd = 0;
    char aCur = 0, bCur = 0;

    while ((aCur = aStr[aInd]) != 0 && (bCur = bStr[bInd]) != 0) {
        if (aCur != bCur) {
            return (int)bCur - (int)aCur;
        }

        aInd++;
        bInd++;
    }

    if (aCur != 0) {  // This all would have been much pretties with macros, but this will probably do
        bInd = 0;

        while ((aCur = aStr[aInd]) != 0 && (bCur = aStr[bInd]) != 0) {
            if (aCur != bCur) {
                return (int)bCur - (int)aCur;
            }

            aInd++;
            bInd++;
        }
    } else if (bCur != 0) {
        aInd = 0;

        while ((aCur = bStr[aInd]) != 0 && (bCur = bStr[bInd]) != 0) {
            if (aCur != bCur) {
                return (int)bCur - (int)aCur;
            }

            aInd++;
            bInd++;
        }
    } else {
        return 0;
    }

    if (aCur == 0) {
        aInd = 0;
    }
    if (bCur == 0) {
        bInd = 0;
    }

    while ((aCur = bStr[aInd]) != 0 && (bCur = aStr[bInd]) != 0) {
        if (aCur != bCur) {
            return (int)bCur - (int)aCur;
        }

        aInd++;
        bInd++;
    }

    return 0;
}


/*

Complexity: O(k * n * log(n)), where n is the number of lines and k is the maximal line length, because we quick sort n lines with a linear comparator

*/

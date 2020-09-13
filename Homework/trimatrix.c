#include <stdio.h>
#include <stdlib.h>

//--------------------------------------------------------------------------------

typedef char * trimatrix;

trimatrix create_tm(size_t length);
char * item_tm(trimatrix tm, size_t y, size_t x);
int read_tm(trimatrix tm, size_t length);
int write_tm(trimatrix tm, size_t length);
int fread_tm(FILE * ifile, trimatrix tm, size_t length);
int fwrite_tm(FILE * ofile, trimatrix tm, size_t length);

//================================================================================

int main(int argc, const char **argv) {
    const size_t len = 5;

    // I'm using the stdin/-out versions of funcs, but this can
    // easily be swapped for the file ones if you'd like
    trimatrix tm = create_tm(len);
    read_tm(tm, len);
    write_tm(tm, len);
    return EXIT_SUCCESS;
}

//================================================================================

trimatrix create_tm(size_t length) {
    return calloc(length * (length + 1) / 2, sizeof(char));
}

char * item_tm(trimatrix tm, size_t y, size_t x) {
    if (x == y) {
        return NULL;
    }
    if (x > y) {
        x = x ^ y;
        y = x ^ y;
        x = x ^ y;
    }
    //printf("(%llu, %llu)", y, x);
    return tm + y * (y + 1) / 2 + x;
}

int read_tm(trimatrix tm, size_t length) {
    fread_tm(stdin, tm, length);
    /*for (size_t y = 0; y <= length; ++y) {
        for (size_t x = 0; x < y; ++x) {
            int cur;
            while (((cur = getchar()) == ' ' || cur == '\n' || cur == '\t') && cur != EOF) {}
            if (cur == EOF) {
                return 1;
            }
            *item_tm(tm, y, x) = (char)cur;
        }
    }
    return 0;*/
}

int write_tm(trimatrix tm, size_t length) {
    fwrite_tm(stdout, tm, length);
    /*for (size_t y = 0; y <= length; ++y) {
        for (size_t x = 0; x < y; ++x) {
            printf("%c ", *item_tm(tm, y, x));
        }
        printf("\n");
    }
    return 0;*/
}

int fread_tm(FILE * ifile, trimatrix tm, size_t length) {
    for (size_t y = 0; y <= length; ++y) {
        for (size_t x = 0; x < y; ++x) {
            int cur;
            while (((cur = getc(ifile)) == ' ' || cur == '\n' || cur == '\t') && cur != EOF) {}
            if (cur == EOF) {
                return 1;
            }
            *item_tm(tm, y, x) = (char)cur;
        }
    }
    return 0;
}

int fwrite_tm(FILE * ofile, trimatrix tm, size_t length) {
    for (size_t y = 0; y <= length; ++y) {
        for (size_t x = 0; x < y; ++x) {
            fprintf(ofile, "%c ", *item_tm(tm, y, x));
        }
        fprintf(ofile, "\n");
    }
    return 0;
}

#include <stdlib.h>
#include <assert.h>
#include <sys/stat.h>

#include "textfile.h"


bool text_read(text_t *self, FILE *ifile) {
    assert(self != NULL);
    assert(ifile != NULL);

    *self = {};

    if (getFSize(ifile, &(self->size))) {
        ERR("Couldn't retrieve file size");
        return true;
    }

    /*if (self->size <= 2) {
        ERR("Empty file");
        return SS_TOOSMALL;
    }*/

    self->buf = (letter_t *)calloc(self->size, sizeof(letter_t));
    if (self->buf == NULL) {
        ERR("Can't allocate space for text");
        return true;
    }

    size_t newSize = fread(self->buf, sizeof(letter_t), self->size - 1, ifile) + 1;

    if (newSize > self->size) {
        ERR("File size increased unexpectedly");
        return true;
    }

    self->size = newSize;

    self->buf[self->size - 1] = '\0';
    if (self->buf[self->size - 2] != '\n') {
        self->buf[self->size - 1] = '\n';
    } else {
        self->size--;
    }

    for (size_t ind = 0; ind < self->size; ++ind) {
        if (self->buf[ind] == '\n') {
            self->buf[ind] = '\0';
            self->length++;
        }
    }

    self->index = (line_t *) calloc(self->length, sizeof(line_t));
    if (self->index == NULL) {
        ERR("Can't allocate space for lines");
        return true;
    }

    size_t offset = 0;
    for (unsigned int lineInd = 0; lineInd < self->length; ++lineInd) {
        if (line_read(&(self->index[lineInd]), self->buf, &offset)) {
            ERR("Can't parse line #%d", lineInd);
            return true;
        }
    }
    assert(offset == self->size);

    return false;
}

bool text_write(text_t *self, FILE *ofile) {
    assert(self != NULL);
    assert(self->buf != NULL);
    assert(self->index != NULL);
    assert(ofile != NULL);

    for (unsigned int ind = 0; ind < self->length; ++ind) {
        if (fputs((const char *)self->index[ind].val, ofile) == EOF) {
            ERR("Can\'t write line #%d", ind);
            return true;
        }

        fputc('\n', ofile);
    }

    return false;
}

bool text_init(text_t *self, unsigned int maxLength, size_t maxSize) {
    assert(self != NULL);

    self->index = (line_t *)calloc(maxLength, sizeof(line_t));
    if (self->index == NULL) {
        ERR("Can't allocate space for lines");
        return true;
    }

    self->buf = (letter_t *)calloc(maxSize, sizeof(letter_t));
    if (self->buf == NULL) {
        ERR("Can't allocate space for text");
        return true;
    }

    self->length = 0;
    self->size = 0;
    return false;
}

void text_free(text_t *self) {
    if (self == NULL) return;

    free(self->index);
    self->index = NULL;

    free(self->buf);
    self->buf = NULL;

    self->length = 0;
    self->size = 0;
}

bool getFSize(FILE *ifile, size_t *length) {
    assert(ifile != NULL);
    assert(length != NULL);

    struct stat buf = {};

    if (fstat(ifile->_file, &buf)) {
        ERR("Couldn't retrieve file size");
        return true;
    }
    *length = buf.st_size + 1;

    return false;
}

bool line_read(line_t *self, letter_t *text, size_t *offset) {
    assert(self != NULL);
    assert(text != NULL);
    assert(offset != NULL);

    self->val = text + *offset;

    for (self->len = 0;
         text[*offset] != '\0' && self->len < MAX_LINE;
         ++*offset, self->len++) {}

    if (text[*offset] != '\0') {
        ERR("Line too long");
        return true;
    }

    self->len++;
    ++*offset;
    return false;
}

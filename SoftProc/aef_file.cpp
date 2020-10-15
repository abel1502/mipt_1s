#include <assert.h>

#include "aef_file.h"
#include "constants.h"

aef_mmap_t *aef_mmap_init(aef_mmap_t *self, code_size_t codeSize, char *code) {
    assert(self != NULL);
    assert(code != NULL);

    self->header.magic = AEF_MAGIC;
    self->header.version = AEF_VERSION;
    self->header.codeSize = codeSize;
    self->header.codeChecksum = crc32_compute(code, codeSize);

    self->code = code;

    self->codeOnHeap = false;

    return self;
}

bool aef_mmap_read(aef_mmap_t *self, FILE *ifile) {
    assert(self != NULL);
    assert(ifile != NULL);

    size_t res = 0;

    res = fread(&self->header, sizeof(self->header), 1, ifile);

    if (res != 1) {
        return true;
    }

    if (self->header.magic != AEF_MAGIC) {
        return true;  // TODO: error codes?
    }

    if (self->header.version != AEF_VERSION) {
        return true;
    }

    self->code = (char *)calloc(self->header.codeSize, sizeof(self->code[0]));

    if (self->code == NULL) {
        return true;
    }

    self->codeOnHeap = true;

    res = fread(self->code, sizeof(self->code[0]), self->header.codeSize, ifile);

    if (res != self->header.codeSize) {
        return true;
    }

    if (crc32_compute(self->code, self->header.codeSize) != self->header.codeChecksum) {
        return true;
    }

    return false;
}

bool aef_mmap_write(aef_mmap_t *self, FILE *ofile) {
    assert(self != NULL);
    assert(ofile != NULL);
    assert(self->code != NULL);

    assert(self->header.magic == AEF_MAGIC);
    assert(self->header.version == AEF_VERSION);
    assert(crc32_compute(self->code, self->header.codeSize) == self->header.codeChecksum);

    size_t res = 0;

    res = fwrite(&self->header, sizeof(self->header), 1, ofile);

    if (res != 1) {
        return true;
    }

    res = fwrite(self->code, sizeof(self->code[0]), self->header.codeSize, ofile);

    if (res != self->header.codeSize) {
        return true;
    }

    return false;
}

void aef_mmap_free(aef_mmap_t *self) {
    assert(self != NULL);

    if (self->codeOnHeap) {
        free(self->code);
    }
}

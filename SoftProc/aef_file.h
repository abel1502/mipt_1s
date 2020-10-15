#ifndef AEF_FILE_H_GUARD
#define AEF_FILE_H_GUARD

#include <stdint.h>
#include <stdio.h>

#include "../libs/checksum.h"
#include "version.h"


typedef struct aef_file_header_s aef_file_header_t;

typedef struct aef_mmap_s aef_mmap_t;

typedef uint32_t code_size_t;


struct aef_file_header_s {
    uint32_t magic;
    version_t version;
    crc32_t codeChecksum;
    code_size_t codeSize;
};

struct aef_mmap_s {
    aef_file_header_t header;
    char *code;
    bool codeOnHeap;
};


aef_mmap_t *aef_mmap_init(aef_mmap_t *self, code_size_t codeSize, char *code);

bool aef_mmap_read(aef_mmap_t *self, FILE *ifile);

bool aef_mmap_write(aef_mmap_t *self, FILE *ofile);

void aef_mmap_free(aef_mmap_t *self);


#endif // AEF_FILE_H_GUARD

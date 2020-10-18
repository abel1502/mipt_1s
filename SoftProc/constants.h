#ifndef CONSTANTS_H_GUARD
#define CONSTANTS_H_GUARD

#include "version.h"

const version_t AEF_VERSION = 2;

// (Due to an 8-year-old bug in gcc, the following pragma does not work.
// #pragma GCC diagnostic ignored "-Wmultichar"
const uint32_t AEF_MAGIC = '\nFEA';  // Abel Executable Format

const unsigned char GENERAL_REG_CNT = 4;  // Warning: Do not increase

#endif // CONSTANTS_H_GUARD

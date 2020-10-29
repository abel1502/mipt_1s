#ifndef CONSTANTS_H_GUARD
#define CONSTANTS_H_GUARD

#include "version.h"

const version_t AEF_VERSION = 4;

// (Due to an 8-year-old bug in gcc, the following pragma does not work.
// #pragma GCC diagnostic ignored "-Wmultichar"
const uint32_t AEF_MAGIC = '\nFEA';  // Abel Executable Format

const unsigned char GENERAL_REG_CNT = 8;

const uint32_t GRAPHICS_SCREEN_HEIGHT = 30;
const uint32_t GRAPHICS_SCREEN_WIDTH = 60;

const uint32_t GRAPHICS_BUF_SIZE = GRAPHICS_SCREEN_HEIGHT * GRAPHICS_SCREEN_WIDTH;

#endif // CONSTANTS_H_GUARD

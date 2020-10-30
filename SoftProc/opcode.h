#ifndef OPCODE_H_GUARD
#define OPCODE_H_GUARD

#include <stdint.h>


#if __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__
#error This can only be compiled on little-endian machines
#endif


typedef union value_u {
    uint64_t qw;
    double df;

    struct {
        float fl;
        float fh;
    };

    struct {
        uint32_t dwl;
        uint32_t dwh;
    };

    struct {
        uint16_t wl;
        uint16_t wh;
    };

    struct {
        uint8_t bl;
        uint8_t bh;
    };
} value_t;

static_assert(sizeof(value_t) == 8);


typedef enum argType_e {
    ARGTYPE_DF  = 0b1110,
    ARGTYPE_FL  = 0b1100,
    ARGTYPE_FH  = 0b1101,
    ARGTYPE_QW  = 0b0110,
    ARGTYPE_DWL = 0b0100,
    ARGTYPE_DWH = 0b0101,
    ARGTYPE_WL  = 0b0010,
    ARGTYPE_WH  = 0b0011,
    ARGTYPE_BL  = 0b0000,
    ARGTYPE_BH  = 0b0001
} __attribute__((__packed__)) argType_t;

static_assert(sizeof(argType_t) == 1);


typedef enum argLoc_e {
    ARGLOC_IMM = 0b001,
    ARGLOC_REG = 0b010,
    ARGLOC_MEM = 0b100
} __attribute__((__packed__)) argLoc_t;

static_assert(sizeof(argLoc_t) == 1);


#pragma pack(1)
typedef struct addrMode_s {
    union {
        uint8_t all : 8;

        struct {
            uint8_t type : 4;
            uint8_t loc : 3;
        };

        struct {
            bool typeLH : 1;  // Inverted because of bitfield order
            uint8_t typeS  : 2;
            bool typeFI : 1;
            bool locImm : 1;
            bool locReg : 1;
            bool locMem : 1;
        };
    };
} addrMode_t;
#pragma pack()

static_assert(sizeof(addrMode_t) == 1);


typedef struct oparg_validityMask_s {
    uint16_t type_mask;
    uint8_t loc_mask;
} oparg_validityMask_t;


#define DEF_OP(NUM, NAME_CAP, NAME_LOW, ARG_CNT, ARG_TYPE_MASK, ARG_LOC_MASK, CODE) \
    OP_##NAME_CAP = NUM,

typedef enum opcode_e {
    #include "opcode_defs.h"
} __attribute__((__packed__)) opcode_t;

static_assert(sizeof(opcode_t) == 1);

#undef DEF_OP


#endif // OPCODE_H_GUARD

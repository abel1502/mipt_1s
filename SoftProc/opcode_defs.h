DEF_OP(0x00, NOP , nop , 0, 0b0000000000000000, 0b00000000, {})

DEF_OP(0x01, PUSH, push, 1, 0b0100000001010101, 0b11101110, {
    PUSH_(ARG_);
})


#define ARGTYPE_CASE_(NAME_CAP, NAME_LOW, TYPE, FMT_U, FMT_S) \
    case ARGTYPE_##NAME_CAP: \
        if (AM_.locMem) { \
            program_ramWriteBytes(self, curOp.memAddr, sizeof(TYPE), &tos0.NAME_LOW); \
        } else { \
            self->registers[curOp.reg].NAME_LOW = tos0.NAME_LOW; \
        } \
        break;
DEF_OP(0x02, POP , pop , 1, 0b0100000001010101, 0b11100100, {
    POP_(&tos0);

    ARGTYPE_SWITCH_(AM_.type,
        ERR("Inexistent argType: 0x%01x", AM_.type);
        return true;
    )
})
#undef ARGTYPE_CASE_

DEF_OP(0x03, POPV, popv, 0, 0b0000000000000000, 0b00000000, {
    POP_(NULL);
})

DEF_OP(0x04, DUP , dup , 0, 0b0000000000000000, 0b00000000, {
    POP_(&tos0);
    PUSH_(tos0);
    PUSH_(tos0);
})

DEF_OP(0x05, ROT , rot , 1, 0b0000000000010101, 0b11101110, {
    switch ((int8_t)ARG_.bl) {
    case 0:
    case 1:
    case -1:
        break;
    case 2:
    case -2:
        POP_(&tos0);
        POP_(&tos1);
        PUSH_(tos0);
        PUSH_(tos1);
        break;
    case 3:
        POP_(&tos0);
        POP_(&tos1);
        POP_(&tos2);
        PUSH_(tos0);
        PUSH_(tos2);
        PUSH_(tos1);
        break;
    case -3:
        POP_(&tos0);
        POP_(&tos1);
        POP_(&tos2);
        PUSH_(tos1);
        PUSH_(tos0);
        PUSH_(tos2);
        break;
    default:
        NOTIMPL_;
    }
})

DEF_OP(0x08, ADD , add , 1, 0b0100000000010000, 0b00000001, {
    POP_(&tos0);
    POP_(&tos1);
    switch (AM_.type) {
    case ARGTYPE_DF:
        res.df = tos1.df + tos0.df;
        break;
    case ARGTYPE_DWL:
        res.dwl = tos1.dwl + tos0.dwl;
        break;
    default:
        NOTIMPL_;
    }
    PUSH_(res);
})

DEF_OP(0x09, SUB , sub , 1, 0b0100000000010000, 0b00000001, {
    POP_(&tos0);
    POP_(&tos1);
    switch (AM_.type) {
    case ARGTYPE_DF:
        res.df = tos1.df - tos0.df;
        break;
    case ARGTYPE_DWL:
        res.dwl = tos1.dwl - tos0.dwl;
        break;
    default:
        NOTIMPL_;
    }
    PUSH_(res);
})

DEF_OP(0x0a, MUL , mul , 1, 0b0100000000010000, 0b00000001, {
    POP_(&tos0);
    POP_(&tos1);
    switch (AM_.type) {
    case ARGTYPE_DF:
        res.df = tos1.df * tos0.df;
        break;
    case ARGTYPE_DWL:
        res.dwl = tos1.dwl * tos0.dwl;
        break;
    default:
        NOTIMPL_;
    }
    PUSH_(res);
})

DEF_OP(0x0b, DIV , div , 1, 0b0100000000010000, 0b00000001, {
    POP_(&tos0);
    POP_(&tos1);
    switch (AM_.type) {
    case ARGTYPE_DF:
        res.df = tos1.df / tos0.df;
        break;
    case ARGTYPE_DWL:
        if (tos0.dwl == 0) {
            ERR("Zero division attempted");
            return true;
        }
        res.dwl = tos1.dwl / tos0.dwl;
        break;
    default:
        NOTIMPL_;
    }
    PUSH_(res);
})

DEF_OP(0x0c, SQR , sqr , 1, 0b0100000000010000, 0b00000001, {
    POP_(&tos0);
    switch (AM_.type) {
    case ARGTYPE_DF:
        res.df = tos0.df * tos0.df;
        break;
    case ARGTYPE_DWL:
        res.dwl = tos0.dwl * tos0.dwl;
        break;
    default:
        NOTIMPL_;
    }
    PUSH_(res);
})

DEF_OP(0x0d, SQRT, sqrt, 1, 0b0100000000000000, 0b00000001, { // Attention: only double
    POP_(&tos0);
    res.df = sqrt(tos0.df);
    PUSH_(res);
})

DEF_OP(0x0e, DEC , dec , 1, 0b0100000000010000, 0b00000001, {
    POP_(&res);
    switch (AM_.type) {
    case ARGTYPE_DF:
        res.df--;
        break;
    case ARGTYPE_DWL:
        res.dwl--;
        break;
    default:
        NOTIMPL_;
    }
    PUSH_(res);
})

DEF_OP(0x0f, INC , inc , 1, 0b0100000000010000, 0b00000001, {
    POP_(&res);
    switch (AM_.type) {
    case ARGTYPE_DF:
        res.df++;
        break;
    case ARGTYPE_DWL:
        res.dwl++;
        break;
    default:
        NOTIMPL_;
    }
    PUSH_(res);
})

DEF_OP(0x10, NEG , neg , 1, 0b0100000000010000, 0b00000001, {
    POP_(&res);
    switch (AM_.type) {
    case ARGTYPE_DF:
        res.df = -res.df;
        break;
    case ARGTYPE_DWL:
        res.dwl = -res.dwl;
        break;
    default:
        NOTIMPL_;
    }
    PUSH_(res);
})

DEF_OP(0x11, MOD , mod , 1, 0b0000000000010000, 0b00000001, {
    POP_(&tos0);
    POP_(&tos1);
    if (tos0.dwl == 0) {
        ERR("Zero division attempted");
        return true;
    }
    res.dwl = tos1.dwl % tos0.dwl;
    PUSH_(res);
})

DEF_OP(0x12, MULS, muls, 1, 0b0000000000010000, 0b00000001, {
    POP_(&tos0);
    POP_(&tos1);
    res.dwl = (uint32_t)((int32_t)tos1.dwl * (int32_t)tos0.dwl);
    PUSH_(res);
})

DEF_OP(0x13, DIVS, divs, 1, 0b0000000000010000, 0b00000001, {
    POP_(&tos0);
    POP_(&tos1);
    if (tos0.dwl == 0) {
        ERR("Zero division attempted");
        return true;
    }
    res.dwl = (uint32_t)((int32_t)tos1.dwl / (int32_t)tos0.dwl);
    PUSH_(res);
})

#define ARGTYPE_CASE_(NAME_CAP, NAME_LOW, TYPE, FMT_U, FMT_S) \
    case ARGTYPE_##NAME_CAP: \
        printf("(%s) > ", #NAME_LOW); \
        if (AM_.locMem) { \
            if (scanf(FMT_U, (TYPE *)program_ramReadBytes(self, curOp.memAddr, sizeof(TYPE), NULL)) != 1) { \
                ERR("Cannot read input"); \
                return true; \
            } \
        } else if (AM_.locReg) { \
            if (scanf(FMT_U, &self->registers[curOp.reg].NAME_LOW) != 1) { \
                ERR("Cannot read input"); \
                return true; \
            } \
        } else { \
            if (scanf(FMT_U, &res.NAME_LOW) != 1) { \
                ERR("Cannot read input"); \
                return true; \
            } \
            PUSH_(res); \
        } \
        break;
DEF_OP(0x18, IN  , in  , 1, 0b0100000001010000, 0b11100101, {
    ARGTYPE_SWITCH_(AM_.type,
        ERR("Inexistent argType: 0x%01x", AM_.type);
        return true;
    )
})
#undef ARGTYPE_CASE_

#define ARGTYPE_CASE_(NAME_CAP, NAME_LOW, TYPE, FMT_U, FMT_S) \
    case ARGTYPE_##NAME_CAP: \
        printf("(%s) ", #NAME_LOW); \
        if (AM_.loc) { \
            printf(FMT_U, ARG_.NAME_LOW); \
        } else { \
            POP_(&tos0); \
            printf(FMT_U, tos0.NAME_LOW); \
        } \
        printf("\n"); \
        break;
DEF_OP(0x19, OUT , out , 1, 0b0100000001010000, 0b11101111, {
    ARGTYPE_SWITCH_(AM_.type,
        ERR("Inexistent argType: 0x%01x", AM_.type);
        return true;
    )
})
#undef ARGTYPE_CASE_

DEF_OP(0x20, JMP , jmp , 1, 0b0000000000010000, 0b11101111, {
    self->ip = ARG_.dwl;
})

DEF_OP(0x21, JF  , jf  , 1, 0b0000000000010000, 0b11101111, {
    POP_(&tos0);
    if (!tos0.dwl) {
        self->ip = ARG_.dwl;
    }
})

DEF_OP(0x22, JT  , jt  , 1, 0b0000000000010000, 0b11101111, {
    POP_(&tos0);
    if (tos0.dwl) {
        self->ip = ARG_.dwl;
    }
})

DEF_OP(0x23, CALL, call, 1, 0b0000000000010000, 0b11101111, {
    res.dwl = self->ip;
    PUSH_FRAME_(res);
    self->ip = ARG_.dwl;
})

DEF_OP(0x24, RET , ret , 0, 0b0000000000000000, 0b00000000, {
    POP_FRAME_(&res);
    self->ip = res.dwl;
})

#define EPSILON_ 1e-12

DEF_OP(0x28, CG  , cg  , 1, 0b0100000000010000, 0b00000001, {
    POP_(&tos0);
    POP_(&tos1);
    switch (AM_.type) {
    case ARGTYPE_DF:
        res.dwl = tos1.df > tos0.df + EPSILON_;
        break;
    case ARGTYPE_DWL:
        res.dwl = (int32_t)tos1.dwl > (int32_t)tos0.dwl;
        break;
    default:
        NOTIMPL_;
    }
    PUSH_(res);
})

DEF_OP(0x29, CGE , cge , 1, 0b0100000000010000, 0b00000001, {
    POP_(&tos0);
    POP_(&tos1);
    switch (AM_.type) {
    case ARGTYPE_DF:
        res.dwl = tos1.df >= tos0.df - EPSILON_;
        break;
    case ARGTYPE_DWL:
        res.dwl = (int32_t)tos1.dwl >= (int32_t)tos0.dwl;
        break;
    default:
        NOTIMPL_;
    }
    PUSH_(res);
})

DEF_OP(0x2a, CL  , cl  , 1, 0b0100000000010000, 0b00000001, {
    POP_(&tos0);
    POP_(&tos1);
    switch (AM_.type) {
    case ARGTYPE_DF:
        res.dwl = tos1.df < tos0.df - EPSILON_;
        break;
    case ARGTYPE_DWL:
        res.dwl = (int32_t)tos1.dwl < (int32_t)tos0.dwl;
        break;
    default:
        NOTIMPL_;
    }
    PUSH_(res);
})

DEF_OP(0x2b, CLE , cle , 1, 0b0100000000010000, 0b00000001, {
    POP_(&tos0);
    POP_(&tos1);
    switch (AM_.type) {
    case ARGTYPE_DF:
        res.dwl = tos1.df <= tos0.df + EPSILON_;
        break;
    case ARGTYPE_DWL:
        res.dwl = (int32_t)tos1.dwl <= (int32_t)tos0.dwl;
        break;
    default:
        NOTIMPL_;
    }
    PUSH_(res);
})

DEF_OP(0x2c, CE  , ce  , 1, 0b0100000000010000, 0b00000001, {
    POP_(&tos0);
    POP_(&tos1);
    switch (AM_.type) {
    case ARGTYPE_DF:
        res.dwl = tos1.df <= tos0.df + EPSILON_ && tos1.df + EPSILON_ >= tos0.df;
        break;
    case ARGTYPE_DWL:
        res.dwl = tos1.dwl == tos0.dwl;
        break;
    default:
        NOTIMPL_;
    }
    PUSH_(res);
})

DEF_OP(0x2d, CNE , cne , 1, 0b0100000000010000, 0b00000001, {
    POP_(&tos0);
    POP_(&tos1);
    switch (AM_.type) {
    case ARGTYPE_DF:
        res.dwl = tos1.df > tos0.df + EPSILON_ || tos1.df + EPSILON_ < tos0.df;
        break;
    case ARGTYPE_DWL:
        res.dwl = tos1.dwl != tos0.dwl;
        break;
    default:
        NOTIMPL_;
    }
    PUSH_(res);
})

#undef EPSILON_

DEF_OP(0xe0, D2B , d2b , 0, 0b0000000000000000, 0b00000000, {
    POP_(&tos0);
    res.bl = (uint8_t)tos0.df;
    PUSH_(res);
})
DEF_OP(0xe1, B2D , b2d , 0, 0b0000000000000000, 0b00000000, {
    POP_(&tos0);
    res.df = (double)tos0.bl;
    PUSH_(res);
})
DEF_OP(0xe2, D2I , d2i , 0, 0b0000000000000000, 0b00000000, {
    POP_(&tos0);
    res.dwl = (int32_t)tos0.df;
    PUSH_(res);
})
DEF_OP(0xe3, I2D , i2d , 0, 0b0000000000000000, 0b00000000, {
    POP_(&tos0);
    res.df = (double)(int32_t)tos0.dwl;
    PUSH_(res);
})

DEF_OP(0xe4, D2U , d2u , 0, 0b0000000000000000, 0b00000000, {
    POP_(&tos0);
    res.dwl = (uint32_t)tos0.df;
    PUSH_(res);
})

DEF_OP(0xe5, U2D , u2d , 0, 0b0000000000000000, 0b00000000, {
    POP_(&tos0);
    res.df = (double)tos0.dwl;
    PUSH_(res);
})

DEF_OP(0xf0, END , end , 0, 0b0000000000000000, 0b00000000, {
    self->flags.flag_exit = true;
})

DEF_OP(0xf1, DUMP, dump, 0, 0b0000000000000000, 0b00000000, {
    program_dump(self);
})

DEF_OP(0xf2, JM  , jm  , 1, 0b0000000000010000, 0b11101110, {
    if (self->flags.flag_monday) {
        self->ip = ARG_.dwl;
    }
})

DEF_OP(0xf3, DRAW, draw, 1, 0b0000000000010000, 0b11101111, {
    if (program_drawScreen(self, ARG_.dwl)) {
        ERR("Failed to draw screen");
        return true;
    }
    Sleep(200);
})

DEF_OP(0xf4, CLDB, cldb, 1, 0b0000000000010000, 0b11101111, {
    memset(program_ramReadBytes(self, ARG_.dwl, GRAPHICS_BUF_SIZE, NULL), ' ', GRAPHICS_BUF_SIZE);
})

DEF_OP(0xf5, CLS , cls , 0, 0b0000000000000000, 0b00000000, {
    system("cls");  // I know it's slow, but so is every other alternative
    //printf("\033[0,0H");
})

DEF_OP(0xf6, SAR , sar , 0, 0b0000000000000000, 0b00000000, {
    for (unsigned ind = 0; ind < GENERAL_REG_CNT; ++ind) {
        PUSH_FRAME_(self->registers[ind]);
    }
})

DEF_OP(0xf7, RAR , rar , 0, 0b0000000000000000, 0b00000000, {
    for (unsigned ind = GENERAL_REG_CNT; ind > 0; --ind) {
        POP_FRAME_(&self->registers[ind - 1]);
    }
})

DEF_OP(0xff, MEOW, meow, 1, 0b0000000000010000, 0b11101111, {
    unsigned count = ARG_.dwl;

    if (!AM_.loc) {
        POP_(&tos0);
        count = tos0.dwl;
    }

    for (unsigned ind = 0; ind < count; ++ind) {
        //txSpeak("ìÿó");
        printf("meow\n");
    }
})

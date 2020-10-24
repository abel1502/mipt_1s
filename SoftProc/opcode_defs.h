DEF_OP(0x00, NOP , nop , 0, 0b0000000000000000, 0b00000000, {})

DEF_OP(0x01, PUSH, push, 1, 0b0100000001010101, 0b00000110, {
    PUSH_(ARG_);
})

DEF_OP(0x02, POP , pop , 1, 0b0100000001010101, 0b00000100, {
    POP_(&self->registers[curOp.reg]);
})

DEF_OP(0x03, POPV, popv, 0, 0b0000000000000000, 0b00000000, {
    POP_(NULL);
})

DEF_OP(0x04, DUP , dup , 0, 0b0000000000000000, 0b00000000, {
    POP_(&tos0);
    PUSH_(tos0);
    PUSH_(tos0);
})

DEF_OP(0x05, ROT , rot , 1, 0b0000000000010000, 0b00000110, {
    switch (ARG_.dwl) {
    case 0:
    case 1:
        break;
    case 2:
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
    default:
        NOTIMPL_;
    }
})

DEF_OP(0x08, ADD , add , 1, 0b0100000000000000, 0b00000001, { TMP_ONLYDOUBLE_
    POP_(&tos0);
	POP_(&tos1);
	res.df = tos1.df + tos0.df;
	PUSH_(res);
})

DEF_OP(0x09, SUB , sub , 1, 0b0100000000000000, 0b00000001, { TMP_ONLYDOUBLE_
    POP_(&tos0);
	POP_(&tos1);
	res.df = tos1.df - tos0.df;
	PUSH_(res);
})

DEF_OP(0x0a, MUL , mul , 1, 0b0100000000000000, 0b00000001, { TMP_ONLYDOUBLE_
    POP_(&tos0);
	POP_(&tos1);
	res.df = tos1.df * tos0.df;
	PUSH_(res);
})

DEF_OP(0x0b, DIV , div , 1, 0b0100000000000000, 0b00000001, { TMP_ONLYDOUBLE_
    POP_(&tos0);
	POP_(&tos1);
	res.df = tos1.df / tos0.df;  // Zero division produces inf
	PUSH_(res);
})

DEF_OP(0x0c, SQR , sqr , 1, 0b0100000000000000, 0b00000001, { TMP_ONLYDOUBLE_
    POP_(&tos0);
	res.df = tos0.df * tos0.df;
	PUSH_(res);
})

DEF_OP(0x0d, SQRT, sqrt, 1, 0b0100000000000000, 0b00000001, { TMP_ONLYDOUBLE_
    POP_(&tos0);
	res.df = sqrt(tos0.df);
	PUSH_(res);
})

DEF_OP(0x18, IN  , in  , 1, 0b0100000000000000, 0b00000101, { TMP_ONLYDOUBLE_
    printf("(df) > ");
    if (AM_.locMem) {
        NOTIMPL_;
    } else if (AM_.locReg) {
        if (scanf("%lg", &self->registers[curOp.reg].df) != 1) {
            ERR("Cannot read input");
            return true;
        }
    } else {
        if (scanf("%lg", &res.df) != 1) {
            ERR("Cannot read input");
            return true;
        }
        PUSH_(res);
    }
})

DEF_OP(0x19, OUT , out , 1, 0b0100000000000000, 0b00000111, { TMP_ONLYDOUBLE_
    printf("(df) ");
    if (AM_.loc) {
        printf("%lg", ARG_.df);
    } else {
        POP_(&tos0);
        printf("%lg", tos0.df);
    }
    printf("\n");
})

DEF_OP(0x20, JMP , jmp , 1, 0b0000000000010000, 0b00000111, {
    self->ip = ARG_.dwl;
})

DEF_OP(0x21, JF  , jf  , 1, 0b0000000000010000, 0b00000111, {
    POP_(&tos0);
    if (!tos0.dwl) {
        self->ip = ARG_.dwl;
    }
})

DEF_OP(0x22, JT  , jt  , 1, 0b0000000000010000, 0b00000111, {
    POP_(&tos0);
    if (tos0.dwl) {
        self->ip = ARG_.dwl;
    }
})

DEF_OP(0x23, CALL, call, 1, 0b0000000000010000, 0b00000111, {  })

DEF_OP(0x24, RET , ret , 0, 0b0000000000000000, 0b00000000, {  })

#define EPSILON_ 1e-12

DEF_OP(0x28, CG  , cg  , 1, 0b0100000000000000, 0b00000001, { TMP_ONLYDOUBLE_
    POP_(&tos0);
	POP_(&tos1);
	res.dwl = tos1.df > tos0.df + EPSILON_;
	PUSH_(res);
})

DEF_OP(0x29, CGE , cge , 1, 0b0100000000000000, 0b00000001, { TMP_ONLYDOUBLE_
    POP_(&tos0);
	POP_(&tos1);
	res.dwl = tos1.df >= tos0.df - EPSILON_;
	PUSH_(res);
})

DEF_OP(0x2a, CL  , cl  , 1, 0b0100000000000000, 0b00000001, { TMP_ONLYDOUBLE_
    POP_(&tos0);
	POP_(&tos1);
	res.dwl = tos1.df < tos0.df - EPSILON_;
	PUSH_(res);
})

DEF_OP(0x2b, CLE , cle , 1, 0b0100000000000000, 0b00000001, { TMP_ONLYDOUBLE_
    POP_(&tos0);
	POP_(&tos1);
	res.dwl = tos1.df <= tos0.df;
	PUSH_(res);
})

DEF_OP(0x2c, CE  , ce  , 1, 0b0100000000000000, 0b00000001, { TMP_ONLYDOUBLE_
    POP_(&tos0);
	POP_(&tos1);
	res.dwl = tos1.df <= tos0.df + EPSILON_ && tos1.df + EPSILON_ >= tos0.df;
	PUSH_(res);
})

DEF_OP(0x2d, CNE , cne , 1, 0b0100000000000000, 0b00000001, { TMP_ONLYDOUBLE_
    POP_(&tos0);
	POP_(&tos1);
	res.dwl = tos1.df > tos0.df + EPSILON_ || tos1.df + EPSILON_ < tos0.df;
	PUSH_(res);
})

#undef EPSILON_

DEF_OP(0xf0, END , end , 0, 0b0000000000000000, 0b00000000, {
    self->flags.flag_exit = true;
})

DEF_OP(0xf1, DUMP, dump, 0, 0b0000000000000000, 0b00000000, {
    program_dump(self);
})


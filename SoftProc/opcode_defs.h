DEF_OP(0x00, NOP , nop , 0, 0b0000000000000000, 0b00000000, {  })

DEF_OP(0x01, PUSH, push, 1, 0b0100000000000000, 0b00000110, {  })

DEF_OP(0x02, POP , pop , 1, 0b0100000000000000, 0b00000100, {  })

DEF_OP(0x03, POPV, popv, 0, 0b0000000000000000, 0b00000000, {  })

DEF_OP(0x04, DUP , dup , 0, 0b0000000000000000, 0b00000000, {  })

DEF_OP(0x05, ROT , rot , 1, 0b0000000000010000, 0b00000110, {  })

DEF_OP(0x08, ADD , add , 1, 0b0100000000000000, 0b00000001, {  })

DEF_OP(0x09, SUB , sub , 1, 0b0100000000000000, 0b00000001, {  })

DEF_OP(0x0a, MUL , mul , 1, 0b0100000000000000, 0b00000001, {  })

DEF_OP(0x0b, DIV , div , 1, 0b0100000000000000, 0b00000001, {  })

DEF_OP(0x0c, SQR , sqr , 1, 0b0100000000000000, 0b00000001, {  })

DEF_OP(0x0d, SQRT, sqrt, 1, 0b0100000000000000, 0b00000001, {  })

DEF_OP(0x18, IN  , in  , 1, 0b0100000000000000, 0b00000101, {
    printf("(df) > ");
    if (AM_.locMem) {
        NOTIMPL_;
    } else if (AM_.locReg) {
        scanf("%lg", &self->registers[curOp.reg].df);
    } else {
        scanf("%lg", &res.df);
        PUSH_(res);
    }
})

DEF_OP(0x19, OUT , out , 1, 0b0100000000000000, 0b00000111, { TMP_ONLYDOUBLE_
    printf("(df) > ");
    if (AM_.loc) {
        printf("%lg", ARG_);
    } else {
        POP_(&tos0);
        printf("%lg", tos0);
    }
    printf("\n");
})

DEF_OP(0x20, JMP , jmp , 1, 0b0000000000010000, 0b00000111, {  })

DEF_OP(0x23, JE  , je  , 1, 0b0000000000010000, 0b00000111, {  })

DEF_OP(0x24, JNE , jne , 1, 0b0000000000010000, 0b00000111, {  })

DEF_OP(0x25, JG  , jg  , 1, 0b0000000000010000, 0b00000111, {  })

DEF_OP(0x26, JGE , jge , 1, 0b0000000000010000, 0b00000111, {  })

DEF_OP(0x27, JL  , jl  , 1, 0b0000000000010000, 0b00000111, {  })

DEF_OP(0x28, JLE , jle , 1, 0b0000000000010000, 0b00000111, {  })

DEF_OP(0xf0, END , end , 0, 0b0000000000000000, 0b00000000, { self->flags.flag_exit = true; })

DEF_OP(0xf1, DUMP, dump, 0, 0b0000000000000000, 0b00000000, { program_dump(self); })


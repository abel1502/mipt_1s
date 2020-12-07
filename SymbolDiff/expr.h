#ifndef EXPR_H
#define EXPR_H

#include "general.h"
#include "vtable.h"
#include "parser.h"

#include <cstdio>


namespace SymbolDiff {

    enum BinOp_e {
        BinOp_Add,
        BinOp_Sub,
        BinOp_Mul,
        BinOp_Div,
        BinOp_Pow,
    };


    enum UnOp_e {
        UnOp_Neg,
        UnOp_Sin,
        UnOp_Cos,
        UnOp_Ln,
    };


    enum Priority_e {
        Priority_Add,
        Priority_Neg,
        Priority_Mul,
        Priority_Ufunc,
        Priority_Pow,
        Priority_Imm,
    };


    class Parser;


    class ExprNode {
    public:
        VTABLE_STRUCT {
            VDECL(ExprNode, void, dtor)
            VDECL(ExprNode, void, dump)
            VDECL(ExprNode, ExprNode *, diff, char by, FILE *logFile)
            VDECL(ExprNode, ExprNode *, copy)
            VDECL(ExprNode, ExprNode *, simplify, bool *wasTrivial)
            VDECL(ExprNode, void, writeTex, FILE *ofile)
            VDECL(ExprNode, Priority_e, getPriority)
            VDECL(ExprNode, bool, isConstBy, char by);
            VDECL(ExprNode, unsigned, getComplexity);
        };

        VTABLE_FIELD

        VTYPE_FIELD(BinOp);
        VTYPE_FIELD(UnOp);
        VTYPE_FIELD(Const);
        VTYPE_FIELD(Var);

        FACTORIES(ExprNode);

        ExprNode *ctor();

        ExprNode *ctorBinOp(BinOp_e new_binOp, ExprNode *new_left, ExprNode *new_right);

        ExprNode *ctorUnOp(UnOp_e new_unOp, ExprNode *new_child);

        ExprNode *ctorVar(char new_varName);

        ExprNode *ctorConst(long long new_value);


        static ExprNode *read(FILE *ifile);

        static ExprNode *read(const char *src);

        static ExprNode *read(Parser *parser);


        void dtor();

        void VMIN(BinOp, dtor)();

        void VMIN(UnOp, dtor)();

        void VMIN(Leaf, dtor)();


        void VMIN(BinOp, dump)();

        void VMIN(UnOp, dump)();

        void VMIN(Const, dump)();

        void VMIN(Var, dump)();


        ExprNode *VMIN(BinOp, diff)(char by, FILE *logFile);

        ExprNode *VMIN(UnOp, diff)(char by, FILE *logFile);

        ExprNode *VMIN(Const, diff)(char by, FILE *logFile);

        ExprNode *VMIN(Var, diff)(char by, FILE *logFile);


        #define CASE_TPL(NAME, STR)  ExprNode *VMIN(BinOp_##NAME, diff)(char by, FILE *logFile);

        #include "tpl_BinOp.h"

        #undef CASE_TPL


        #define CASE_TPL(NAME, STR)  ExprNode *VMIN(UnOp_##NAME, diff)(char by, FILE *logFile);

        #include "tpl_UnOp.h"

        #undef CASE_TPL


        ExprNode *VMIN(BinOp, copy)();

        ExprNode *VMIN(UnOp, copy)();

        ExprNode *VMIN(Const, copy)();

        ExprNode *VMIN(Var, copy)();


        ExprNode *VMIN(BinOp, simplify)(bool *wasTrivial);

        ExprNode *VMIN(UnOp, simplify)(bool *wasTrivial);

        ExprNode *VMIN(Leaf, simplify)(bool *wasTrivial);


        #define CASE_TPL(NAME, STR)  ExprNode *VMIN(BinOp_##NAME, simplify)(bool *wasTrivial);

        #include "tpl_BinOp.h"

        #undef CASE_TPL


        #define CASE_TPL(NAME, STR)  ExprNode *VMIN(UnOp_##NAME, simplify)(bool *wasTrivial);

        #include "tpl_UnOp.h"

        #undef CASE_TPL


        void VMIN(BinOp, writeTex)(FILE *ofile);

        void VMIN(UnOp, writeTex)(FILE *ofile);

        void VMIN(Const, writeTex)(FILE *ofile);

        void VMIN(Var, writeTex)(FILE *ofile);


        Priority_e VMIN(BinOp, getPriority)();

        Priority_e VMIN(UnOp, getPriority)();

        Priority_e VMIN(Const, getPriority)();

        Priority_e VMIN(Var, getPriority)();


        bool VMIN(BinOp, isConstBy)(char by);

        bool VMIN(UnOp, isConstBy)(char by);

        bool VMIN(Const, isConstBy)(char by);

        bool VMIN(Var, isConstBy)(char by);


        unsigned VMIN(BinOp, getComplexity)();

        unsigned VMIN(UnOp, getComplexity)();

        unsigned VMIN(Const, getComplexity)();

        unsigned VMIN(Var, getComplexity)();

    private:
        union {
            // Binary Op
            struct {
                ExprNode *children[2];
                BinOp_e binOp;
            };

            struct {
                ExprNode *left;
                ExprNode *right;
            };

            // Unary Op
            struct {
                ExprNode *child;
                UnOp_e unOp;
            };

            // Constant
            struct {
                long long value;
            };

            // Variable
            struct {
                char varName;
            };
        };


        static constexpr ExprNode *(ExprNode::* const binOpDifferentiators[])(char by, FILE *logFile) = {
            #define CASE_TPL(NAME, STR)  [BinOp_##NAME] = ExprNode::VMIN(BinOp_##NAME, diff),

            #include "tpl_BinOp.h"

            #undef CASE_TPL
        };

        static constexpr ExprNode *(ExprNode::* const unOpDifferentiators[])(char by, FILE *logFile) = {
            #define CASE_TPL(NAME, STR)  [UnOp_##NAME] = ExprNode::VMIN(UnOp_##NAME, diff),

            #include "tpl_UnOp.h"

            #undef CASE_TPL
        };

        static constexpr ExprNode *(ExprNode::* const binOpSimplifiers[])(bool *wasTrivial) = {
            #define CASE_TPL(NAME, STR)  [BinOp_##NAME] = ExprNode::VMIN(BinOp_##NAME, simplify),

            #include "tpl_BinOp.h"

            #undef CASE_TPL
        };

        static constexpr ExprNode *(ExprNode::* const unOpSimplifiers[])(bool *wasTrivial) = {
            #define CASE_TPL(NAME, STR)  [UnOp_##NAME] = ExprNode::VMIN(UnOp_##NAME, simplify),

            #include "tpl_UnOp.h"

            #undef CASE_TPL
        };


        static const unsigned OPSTR_MAX_LEN = 8;
        typedef char OpStr[OPSTR_MAX_LEN + 1];

        static const constexpr OpStr binOpStrings[] = {
            #define CASE_TPL(NAME, STR)  STR,

            #include "tpl_BinOp.h"

            #undef CASE_TPL
        };

        static const constexpr OpStr unOpStrings[] = {
            #define CASE_TPL(NAME, STR)  STR,

            #include "tpl_UnOp.h"

            #undef CASE_TPL
        };

    };


    class ExprTree {
    public:
        FACTORIES(ExprTree);

        ExprTree *ctor();

        void dtor();

        bool read(FILE *ifile);

        bool read(const char *src);

        void writeTex(FILE *ofile);

        void dump();

        void simplify();

        ExprTree *diff(char by='x');

    private:
        ExprNode *root;
    };

}

#endif // EXPR_H

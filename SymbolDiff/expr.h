#ifndef EXPR_H
#define EXPR_H

#include "general.h"
#include "vtable.h"


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
    };


    class ExprNode {
    public:
        VTABLE_STRUCT {
            VDECL(ExprNode, void, dtor)
            VDECL(ExprNode, void, dump)
            VDECL(ExprNode, ExprNode *, diff)
            VDECL(ExprNode, ExprNode *, copy)
            VDECL(ExprNode, ExprNode *, simplify, bool *)
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


        void dtor();

        void VMIN(BinOp, dtor)();

        void VMIN(UnOp, dtor)();

        void VMIN(Leaf, dtor)();


        void VMIN(BinOp, dump)();

        void VMIN(UnOp, dump)();

        void VMIN(Const, dump)();

        void VMIN(Var, dump)();


        ExprNode *VMIN(BinOp, diff)();

        ExprNode *VMIN(UnOp, diff)();

        ExprNode *VMIN(Const, diff)();

        ExprNode *VMIN(Var, diff)();


        ExprNode *VMIN(BinOp_Add, diff)();

        ExprNode *VMIN(BinOp_Sub, diff)();

        ExprNode *VMIN(BinOp_Mul, diff)();

        ExprNode *VMIN(BinOp_Div, diff)();

        ExprNode *VMIN(BinOp_Pow, diff)();


        ExprNode *VMIN(UnOp_Neg, diff)();

        ExprNode *VMIN(UnOp_Sin, diff)();

        ExprNode *VMIN(UnOp_Cos, diff)();


        ExprNode *VMIN(BinOp, copy)();

        ExprNode *VMIN(UnOp, copy)();

        ExprNode *VMIN(Const, copy)();

        ExprNode *VMIN(Var, copy)();


        ExprNode *VMIN(BinOp, simplify)(bool *wasTrivial);

        ExprNode *VMIN(UnOp, simplify)(bool *wasTrivial);

        ExprNode *VMIN(Leaf, simplify)(bool *wasTrivial);


        ExprNode *VMIN(BinOp_Add, simplify)(bool *wasTrivial);

        ExprNode *VMIN(BinOp_Sub, simplify)(bool *wasTrivial);

        ExprNode *VMIN(BinOp_Mul, simplify)(bool *wasTrivial);

        ExprNode *VMIN(BinOp_Div, simplify)(bool *wasTrivial);

        ExprNode *VMIN(BinOp_Pow, simplify)(bool *wasTrivial);


        ExprNode *VMIN(UnOp_Neg, simplify)(bool *wasTrivial);

        ExprNode *VMIN(UnOp_Sin, simplify)(bool *wasTrivial);

        ExprNode *VMIN(UnOp_Cos, simplify)(bool *wasTrivial);


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

        static ExprNode *(ExprNode::* const binOpDifferentiators[])();

        static ExprNode *(ExprNode::* const unOpDifferentiators[])();

        static ExprNode *(ExprNode::* const binOpSimplifiers[])(bool *wasTrivial);

        static ExprNode *(ExprNode::* const unOpSimplifiers[])(bool *wasTrivial);


        static const unsigned OPSTR_MAX_LEN = 8;
        typedef char opStr[OPSTR_MAX_LEN];

        static const constexpr opStr binOpStrings[] = {
            "+",
            "-",
            "*",
            "/",
            "^",  // "**"?
        };

        static const constexpr opStr unOpStrings[] = {
            "-",
            "sin",
            "cos",
        };
    };


    class ExprTree {
    };

}

#endif // EXPR_H

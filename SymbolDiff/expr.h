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
    };


    enum UnOp_e {
        UnOp_Pos,
        UnOp_Neg,
        UnOp_Sin,
        UnOp_Cos,
    };


    class ExprNode {
    public:
        VTABLE_STRUCT {
            VDECL(ExprNode, void, dtor)
            VDECL(ExprNode, void, dump)
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


        ExprNode *VMIN(UnOp_Pos, diff)();

        ExprNode *VMIN(UnOp_Neg, diff)();

        ExprNode *VMIN(UnOp_Sin, diff)();

        ExprNode *VMIN(UnOp_Cos, diff)();

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
    };


    class ExprTree {
    };

}

#endif // EXPR_H

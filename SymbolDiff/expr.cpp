#include "expr.h"

#include <cstdio>
#include <cstdlib>
#include <cassert>


namespace SymbolDiff {

    ExprNode *ExprNode::ctor() {
        return this;  // TODO: Zero-fill?
    }

    ExprNode *ExprNode::ctorBinOp(BinOp_e new_binOp, ExprNode *new_left, ExprNode *new_right) {
        VSETTYPE(this, BinOp);

        assert(new_left && new_right);

        binOp = new_binOp;
        left = new_left;
        right = new_right;

        return this;
    }

    ExprNode *ExprNode::ctorUnOp(UnOp_e new_unOp, ExprNode *new_child) {
        VSETTYPE(this, UnOp);

        assert(new_child);

        unOp = new_unOp;
        child = new_child;

        return this;
    }

    ExprNode *ExprNode::ctorVar(char new_varName) {
        VSETTYPE(this, Var);

        varName = new_varName;

        return this;
    }

    ExprNode *ExprNode::ctorConst(long long new_value) {
        VSETTYPE(this, Const);

        value = new_value;

        return this;
    }


    void ExprNode::dtor() {
        VCALL(this, dtor);
    }

    //--------------------------------------------------------------------------------

    void ExprNode::VMIN(BinOp, dump)() {
        printf("(");
        VCALL(left, dump);
        printf(" %c ", "+-*/"[binOp]);  // TODO: Replace
        VCALL(right, dump);
        printf(")");
    }

    void ExprNode::VMIN(UnOp, dump)() {
        printf("(%c (", "+-"[unOp]);    // TODO: Replace
        VCALL(child, dump);
        printf(")");
    }

    void ExprNode::VMIN(Const, dump)() {
        printf("(%lld)", value);
    }

    void ExprNode::VMIN(Var, dump)() {
        printf("(%c)", varName);
    }


    void ExprNode::VMIN(BinOp, dtor)() {
        VCALL(left, dtor);
        delete left;
        VCALL(right, dtor);
        delete right;
    }

    void ExprNode::VMIN(UnOp, dtor)() {
        VCALL(child, dtor);
        delete child;
    }

    void ExprNode::VMIN(Leaf, dtor)() {}


    ExprNode *ExprNode::VMIN(BinOp, diff)() {
        return (this->*binOpDifferentiators[binOp])();
    }

    ExprNode *ExprNode::VMIN(UnOp, diff)() {
        return (this->*unOpDifferentiators[binOp])();
    }

    ExprNode *ExprNode::VMIN(Const, diff)() {
        return ExprNode::create()->ctorConst(0);
    }

    ExprNode *ExprNode::VMIN(Var, diff)() {
        return ExprNode::create()->ctorConst(1);
    }


    ExprNode *ExprNode::VMIN(BinOp_Add, diff)() {
    }

    ExprNode *ExprNode::VMIN(BinOp_Sub, diff)() {
    }

    ExprNode *ExprNode::VMIN(BinOp_Mul, diff)() {
    }

    ExprNode *ExprNode::VMIN(BinOp_Div, diff)() {
    }


    ExprNode *ExprNode::VMIN(UnOp_Pos, diff)() {
    }

    ExprNode *ExprNode::VMIN(UnOp_Neg, diff)() {
    }

    ExprNode *ExprNode::VMIN(UnOp_Sin, diff)() {
    }

    ExprNode *ExprNode::VMIN(UnOp_Cos, diff)() {
    }

    //--------------------------------------------------------------------------------

    ExprNode *(ExprNode::* const ExprNode::binOpDifferentiators[])() = {
        [BinOp_Add] = ExprNode::VMIN(BinOp_Add, diff),
        [BinOp_Sub] = ExprNode::VMIN(BinOp_Sub, diff),
        [BinOp_Mul] = ExprNode::VMIN(BinOp_Mul, diff),
        [BinOp_Div] = ExprNode::VMIN(BinOp_Div, diff),
    };

    ExprNode *(ExprNode::* const ExprNode::unOpDifferentiators[])() = {
        [UnOp_Pos] = ExprNode::VMIN(UnOp_Pos, diff),
        [UnOp_Neg] = ExprNode::VMIN(UnOp_Neg, diff),
        [UnOp_Sin] = ExprNode::VMIN(UnOp_Sin, diff),
        [UnOp_Cos] = ExprNode::VMIN(UnOp_Cos, diff),
    };

    //--------------------------------------------------------------------------------

    VTYPE_DEF(BinOp, ExprNode) = {
        ExprNode::VMIN(BinOp, dtor),
        ExprNode::VMIN(BinOp, dump),
    };

    VTYPE_DEF(UnOp, ExprNode) = {
        ExprNode::VMIN(UnOp, dtor),
        ExprNode::VMIN(UnOp, dump),
    };

    VTYPE_DEF(Const, ExprNode) = {
        ExprNode::VMIN(Leaf, dtor),
        ExprNode::VMIN(Const, dump),
    };

    VTYPE_DEF(Var, ExprNode) = {
        ExprNode::VMIN(Leaf, dtor),
        ExprNode::VMIN(Var, dump),
    };

}


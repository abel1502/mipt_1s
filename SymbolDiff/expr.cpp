#include "expr.h"

#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <cmath>


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
        printf(" %c ", "+-*/^"[binOp]);  // TODO: Replace
        VCALL(right, dump);
        printf(")");
    }

    void ExprNode::VMIN(UnOp, dump)() {
        printf("(%c (", "-sc"[unOp]);    // TODO: Replace
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
        if (left) {
            VCALL(left, dtor);
            delete left;
        }

        if (right) {
            VCALL(right, dtor);
            delete right;
        }
    }

    void ExprNode::VMIN(UnOp, dtor)() {
        if (child) {
            VCALL(child, dtor);
            delete child;
        }
    }

    void ExprNode::VMIN(Leaf, dtor)() {}


    // Differentiation DSL begin

    #define ADD_(A, B)  ExprNode::create()->ctorBinOp(BinOp_Add, A, B)
    #define SUB_(A, B)  ExprNode::create()->ctorBinOp(BinOp_Sub, A, B)
    #define MUL_(A, B)  ExprNode::create()->ctorBinOp(BinOp_Mul, A, B)
    #define DIV_(A, B)  ExprNode::create()->ctorBinOp(BinOp_Div, A, B)
    #define POW_(A, B)  ExprNode::create()->ctorBinOp(BinOp_Pow, A, B)

    #define NEG_(A)     ExprNode::create()->ctorUnOp(UnOp_Neg, A)
    #define SIN_(A)     ExprNode::create()->ctorUnOp(UnOp_Sin, A)
    #define COS_(A)     ExprNode::create()->ctorUnOp(UnOp_Cos, A)

    #define COPY_(A)    VCALL(A, copy)
    #define DIFF_(A)    VCALL(A, diff)
    #define CONST_(A)   ExprNode::create()->ctorConst(A)
    #define VAR_(A)     ExprNode::create()->ctorVar(A)


    ExprNode *ExprNode::VMIN(BinOp, diff)() {
        return (this->*binOpDifferentiators[binOp])();
    }

    ExprNode *ExprNode::VMIN(UnOp, diff)() {
        return (this->*unOpDifferentiators[unOp])();
    }

    ExprNode *ExprNode::VMIN(Const, diff)() {
        return CONST_(0);
    }

    ExprNode *ExprNode::VMIN(Var, diff)() {
        assert(varName == 'x');

        return CONST_(1);
    }


    ExprNode *ExprNode::VMIN(BinOp_Add, diff)() {
        return ADD_(DIFF_(left), DIFF_(right));
    }

    ExprNode *ExprNode::VMIN(BinOp_Sub, diff)() {
        return SUB_(DIFF_(left), DIFF_(right));
    }

    ExprNode *ExprNode::VMIN(BinOp_Mul, diff)() {
        return ADD_(MUL_(DIFF_(left), COPY_(right)), MUL_(COPY_(left), DIFF_(right)));
    }

    ExprNode *ExprNode::VMIN(BinOp_Div, diff)() {
        return DIV_(SUB_(MUL_(DIFF_(left), COPY_(right)), MUL_(COPY_(left), DIFF_(right))), POW_(COPY_(right), CONST_(2)));
    }
    ExprNode *ExprNode::VMIN(BinOp_Pow, diff)() {
        assert(VISINST(right, Const));

        return MUL_(MUL_(COPY_(right), POW_(COPY_(left), SUB_(COPY_(right), CONST_(1)))), DIFF_(left));
    }


    ExprNode *ExprNode::VMIN(UnOp_Neg, diff)() {
        return NEG_(DIFF_(child));
    }

    ExprNode *ExprNode::VMIN(UnOp_Sin, diff)() {
        return MUL_(COS_(COPY_(child)), DIFF_(child));
    }

    ExprNode *ExprNode::VMIN(UnOp_Cos, diff)() {
        return NEG_(MUL_(SIN_(COPY_(child)), DIFF_(child)));
    }


    ExprNode *ExprNode::VMIN(BinOp, simplify)(bool *wasTrivial) {
        /*printf("!");
        VCALL(this, dump);
        printf("\n");*/

        *wasTrivial = false;
        while (!*wasTrivial)
            left = VCALL(left, simplify, wasTrivial);

        *wasTrivial = false;
        while (!*wasTrivial)
            right = VCALL(right, simplify, wasTrivial);

        /*printf(">");
        VCALL(this, dump);
        printf("\n");*/

        return (this->*binOpSimplifiers[binOp])(wasTrivial);
    }

    ExprNode *ExprNode::VMIN(UnOp, simplify)(bool *wasTrivial) {
        *wasTrivial = false;
        while (!*wasTrivial)
            child = VCALL(child, simplify, wasTrivial);

        return (this->*unOpSimplifiers[unOp])(wasTrivial);
    }

    ExprNode *ExprNode::VMIN(Leaf, simplify)(bool *wasTrivial) {
        *wasTrivial = true;

        return this;
    }


    ExprNode *ExprNode::VMIN(BinOp_Add, simplify)(bool *wasTrivial) {
        if (VISINST(left, Const) && VISINST(right, Const)) {
            ExprNode *tmp = left;

            tmp->value += right->value;

            left = nullptr;
            dtor();
            delete this;

            return tmp;
        }

        for (int i = 0; i < 2; ++i) {
            if (VISINST(children[i], Const) && children[i]->value == 0) {
                ExprNode *tmp = children[!i];

                children[!i] = nullptr;
                dtor();
                delete this;

                return tmp;
            }
        }

        *wasTrivial = true;

        return this;
    }

    ExprNode *ExprNode::VMIN(BinOp_Sub, simplify)(bool *wasTrivial) {
        if (VISINST(left, Const) && VISINST(right, Const)) {
            ExprNode *tmp = left;

            tmp->value -= right->value;

            left = nullptr;
            dtor();
            delete this;

            return tmp;
        }

        if (VISINST(left, Const) && left->value == 0) {
            left->dtor();
            delete left;

            ctorUnOp(UnOp_Neg, right);

            return this;
        }

        if (VISINST(right, Const) && right->value == 0) {
            ExprNode *tmp = left;

            left = nullptr;
            dtor();
            delete this;

            return tmp;
        }

        *wasTrivial = true;

        return this;
    }

    ExprNode *ExprNode::VMIN(BinOp_Mul, simplify)(bool *wasTrivial) {
        if (VISINST(left, Const) && VISINST(right, Const)) {
            ExprNode *tmp = left;

            tmp->value *= right->value;

            left = nullptr;
            dtor();
            delete this;

            return tmp;
        }

        for (int i = 0; i < 2; ++i) {
            if (VISINST(children[i], Const)) {
                ExprNode *tmp = nullptr;

                switch (children[i]->value) {
                case 0:
                    dtor();
                    delete this;

                    return CONST_(0);
                case 1:
                    tmp = children[!i];

                    children[!i] = nullptr;
                    dtor();
                    delete this;

                    return tmp;
                case -1:
                    children[i]->dtor();
                    delete children[i];

                    ctorUnOp(UnOp_Neg, children[!i]);

                    return this;
                default:
                    break;
                }
            }
        }

        *wasTrivial = true;

        return this;
    }

    ExprNode *ExprNode::VMIN(BinOp_Div, simplify)(bool *wasTrivial) {
        if (VISINST(left, Const) && VISINST(right, Const) && right != 0 && left->value % right->value == 0) {
            ExprNode *tmp = left;

            tmp->value /= right->value;

            left = nullptr;
            dtor();
            delete this;

            return tmp;
        }

        if (VISINST(right, Const) && right->value == 1) {
            ExprNode *tmp = left;

            left = nullptr;
            dtor();
            delete this;

            return tmp;
        }

        *wasTrivial = true;

        return this;
    }

    ExprNode *ExprNode::VMIN(BinOp_Pow, simplify)(bool *wasTrivial) {
        if (VISINST(left, Const) && VISINST(right, Const)) {
            ExprNode *tmp = left;

            tmp->value = pow(tmp->value, right->value);

            left = nullptr;
            dtor();
            delete this;

            return tmp;
        }

        if (VISINST(right, Const) && right->value == 1) {
            ExprNode *tmp = left;

            left = nullptr;
            dtor();
            delete this;

            return tmp;
        }

        if (VISINST(left, Const) && left->value == 1) {
            dtor();
            delete this;

            return CONST_(1);
        }

        *wasTrivial = true;

        return this;
    }


    ExprNode *ExprNode::VMIN(UnOp_Neg, simplify)(bool *wasTrivial) {
        if (VISINST(child, UnOp) && child->unOp == UnOp_Neg) {
            ExprNode *tmp = child->child;

            child->child = nullptr;
            dtor();
            delete this;

            return tmp;
        }

        if (VISINST(child, Const)) {
            ExprNode *tmp = child;

            child = nullptr;
            dtor();
            delete this;

            tmp->value = -tmp->value;

            return tmp;
        }

        *wasTrivial = true;

        return this;
    }

    ExprNode *ExprNode::VMIN(UnOp_Sin, simplify)(bool *wasTrivial) {
        *wasTrivial = true;

        return this;
    }

    ExprNode *ExprNode::VMIN(UnOp_Cos, simplify)(bool *wasTrivial) {
        *wasTrivial = true;

        return this;
    }

    #undef ADD_
    #undef SUB_
    #undef MUL_
    #undef DIV_
    #undef POW_

    #undef NEG_
    #undef SIN_
    #undef COS_

    #undef COPY_
    #undef DIFF_
    #undef CONST_
    #undef VAR_

    // Differentiation DSL end


    ExprNode *ExprNode::VMIN(BinOp, copy)() {
        return ExprNode::create()->ctorBinOp(binOp, VCALL(left, copy), VCALL(right, copy));
    }

    ExprNode *ExprNode::VMIN(UnOp, copy)() {
        return ExprNode::create()->ctorUnOp(unOp, VCALL(child, copy));
    }

    ExprNode *ExprNode::VMIN(Const, copy)() {
        return ExprNode::create()->ctorConst(value);
    }

    ExprNode *ExprNode::VMIN(Var, copy)() {
        return ExprNode::create()->ctorVar(varName);
    }

    //--------------------------------------------------------------------------------

    ExprNode *(ExprNode::* const ExprNode::binOpDifferentiators[])() = {
        [BinOp_Add] = ExprNode::VMIN(BinOp_Add, diff),
        [BinOp_Sub] = ExprNode::VMIN(BinOp_Sub, diff),
        [BinOp_Mul] = ExprNode::VMIN(BinOp_Mul, diff),
        [BinOp_Div] = ExprNode::VMIN(BinOp_Div, diff),
        [BinOp_Pow] = ExprNode::VMIN(BinOp_Pow, diff),
    };

    ExprNode *(ExprNode::* const ExprNode::unOpDifferentiators[])() = {
        [UnOp_Neg] = ExprNode::VMIN(UnOp_Neg, diff),
        [UnOp_Sin] = ExprNode::VMIN(UnOp_Sin, diff),
        [UnOp_Cos] = ExprNode::VMIN(UnOp_Cos, diff),
    };

    ExprNode *(ExprNode::* const ExprNode::binOpSimplifiers[])(bool *wasTrivial) = {
        [BinOp_Add] = ExprNode::VMIN(BinOp_Add, simplify),
        [BinOp_Sub] = ExprNode::VMIN(BinOp_Sub, simplify),
        [BinOp_Mul] = ExprNode::VMIN(BinOp_Mul, simplify),
        [BinOp_Div] = ExprNode::VMIN(BinOp_Div, simplify),
        [BinOp_Pow] = ExprNode::VMIN(BinOp_Pow, simplify),
    };

    ExprNode *(ExprNode::* const ExprNode::unOpSimplifiers[])(bool *wasTrivial) = {
        [UnOp_Neg] = ExprNode::VMIN(UnOp_Neg, simplify),
        [UnOp_Sin] = ExprNode::VMIN(UnOp_Sin, simplify),
        [UnOp_Cos] = ExprNode::VMIN(UnOp_Cos, simplify),
    };

    //--------------------------------------------------------------------------------

    VTYPE_DEF(BinOp, ExprNode) = {
        ExprNode::VMIN(BinOp, dtor),
        ExprNode::VMIN(BinOp, dump),
        ExprNode::VMIN(BinOp, diff),
        ExprNode::VMIN(BinOp, copy),
        ExprNode::VMIN(BinOp, simplify),
    };

    VTYPE_DEF(UnOp, ExprNode) = {
        ExprNode::VMIN(UnOp, dtor),
        ExprNode::VMIN(UnOp, dump),
        ExprNode::VMIN(UnOp, diff),
        ExprNode::VMIN(UnOp, copy),
        ExprNode::VMIN(UnOp, simplify),
    };

    VTYPE_DEF(Const, ExprNode) = {
        ExprNode::VMIN(Leaf, dtor),
        ExprNode::VMIN(Const, dump),
        ExprNode::VMIN(Const, diff),
        ExprNode::VMIN(Const, copy),
        ExprNode::VMIN(Leaf, simplify),
    };

    VTYPE_DEF(Var, ExprNode) = {
        ExprNode::VMIN(Leaf, dtor),
        ExprNode::VMIN(Var, dump),
        ExprNode::VMIN(Var, diff),
        ExprNode::VMIN(Var, copy),
        ExprNode::VMIN(Leaf, simplify),
    };

}


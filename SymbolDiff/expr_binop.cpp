#include "expr.h"

#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <cmath>


namespace SymbolDiff {

    ExprNode *ExprNode::ctorBinOp(BinOp_e new_binOp, ExprNode *new_left, ExprNode *new_right) {
        VSETTYPE(this, BinOp);

        assert(new_left && new_right);

        binOp = new_binOp;
        left = new_left;
        right = new_right;

        return this;
    }

    void ExprNode::VMIN(BinOp, dump)() {
        printf("(");
        VCALL(left, dump);
        printf(" %s ", binOpStrings[binOp]);
        VCALL(right, dump);
        printf(")");
    }

    void ExprNode::VMIN(BinOp, dtor)() {
        if (left) {
            left->destroy();
        }

        if (right) {
            right->destroy();
        }
    }

    #include "expr_dsl_def.h"

    ExprNode *ExprNode::VMIN(BinOp, diff)(char by) {
        return (this->*binOpDifferentiators[binOp])(by);
    }


    ExprNode *ExprNode::VMIN(BinOp_Add, diff)(char by) {
        return ADD_(DIFF_(left), DIFF_(right));
    }

    ExprNode *ExprNode::VMIN(BinOp_Sub, diff)(char by) {
        return SUB_(DIFF_(left), DIFF_(right));
    }

    ExprNode *ExprNode::VMIN(BinOp_Mul, diff)(char by) {
        return ADD_(MUL_(DIFF_(left), COPY_(right)), MUL_(COPY_(left), DIFF_(right)));
    }

    ExprNode *ExprNode::VMIN(BinOp_Div, diff)(char by) {
        return DIV_(SUB_(MUL_(DIFF_(left), COPY_(right)), MUL_(COPY_(left), DIFF_(right))), POW_(COPY_(right), CONST_(2)));
    }

    ExprNode *ExprNode::VMIN(BinOp_Pow, diff)(char by) {
        assert(VISINST(right, Const));

        return MUL_(MUL_(COPY_(right), POW_(COPY_(left), SUB_(COPY_(right), CONST_(1)))), DIFF_(left));
    }


    ExprNode *ExprNode::VMIN(BinOp, simplify)(bool *wasTrivial) {
        *wasTrivial = false;
        while (!*wasTrivial)
            left = VCALL(left, simplify, wasTrivial);

        *wasTrivial = false;
        while (!*wasTrivial)
            right = VCALL(right, simplify, wasTrivial);

        return (this->*binOpSimplifiers[binOp])(wasTrivial);
    }


    ExprNode *ExprNode::VMIN(BinOp_Add, simplify)(bool *wasTrivial) {
        if (VISINST(left, Const) && VISINST(right, Const)) {
            ExprNode *tmp = left;

            tmp->value += right->value;

            left = nullptr;
            destroy();

            return tmp;
        }

        for (int i = 0; i < 2; ++i) {
            if (VISINST(children[i], Const) && children[i]->value == 0) {
                ExprNode *tmp = children[!i];

                children[!i] = nullptr;
                destroy();

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
            destroy();

            return tmp;
        }

        if (VISINST(left, Const) && left->value == 0) {
            left->destroy();

            ctorUnOp(UnOp_Neg, right);

            return this;
        }

        if (VISINST(right, Const) && right->value == 0) {
            ExprNode *tmp = left;

            left = nullptr;
            destroy();

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
            destroy();

            return tmp;
        }

        for (int i = 0; i < 2; ++i) {
            if (VISINST(children[i], Const)) {
                ExprNode *tmp = nullptr;

                switch (children[i]->value) {
                case 0:
                    tmp = children[i];

                    children[i] = nullptr;
                    destroy();

                    return tmp;
                case 1:
                    tmp = children[!i];

                    children[!i] = nullptr;
                    destroy();

                    return tmp;
                case -1:
                    children[i]->destroy();
                    children[i] = nullptr;  // Just in case

                    return ctorUnOp(UnOp_Neg, children[!i]);
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
            destroy();

            return tmp;
        }

        if (VISINST(right, Const) && right->value == 1) {
            ExprNode *tmp = left;

            left = nullptr;
            destroy();

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
            destroy();

            return tmp;
        }

        if (VISINST(right, Const) && right->value == 1) {
            ExprNode *tmp = left;

            left = nullptr;
            destroy();

            return tmp;
        }

        if (VISINST(left, Const) && left->value == 1) {
            ExprNode *tmp = left;

            left = nullptr;
            destroy();

            return tmp;
        }

        // Order matters, since 0**0 == 1
        if (VISINST(right, Const) && right->value == 0) {
            ExprNode *tmp = right;

            right->value = 1;

            right = nullptr;
            destroy();

            return tmp;
        }

        if (VISINST(left, Const) && left->value == 0) {
            ExprNode *tmp = left;

            left = nullptr;
            destroy();

            return tmp;
        }

        *wasTrivial = true;

        return this;
    }

    #include "expr_dsl_undef.h"


    ExprNode *ExprNode::VMIN(BinOp, copy)() {
        return ExprNode::create()->ctorBinOp(binOp, VCALL(left, copy), VCALL(right, copy));
    }

    //--------------------------------------------------------------------------------

    VTYPE_DEF(BinOp, ExprNode) = {
        ExprNode::VMIN(BinOp, dtor),
        ExprNode::VMIN(BinOp, dump),
        ExprNode::VMIN(BinOp, diff),
        ExprNode::VMIN(BinOp, copy),
        ExprNode::VMIN(BinOp, simplify),
    };
}


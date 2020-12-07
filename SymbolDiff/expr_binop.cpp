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

    ExprNode *ExprNode::VMIN(BinOp, diff)(char by, FILE *logFile) {
        ExprNode *result = (this->*binOpDifferentiators[binOp])(by, logFile);

        TEXP("So, this subexpression results in:\\par\n"
             "$ ");
        result->writeTex(logFile);
        TEXP(" $\\par\n");

        int cycles = 0;
        bool wasTrivial = false;
        while (!wasTrivial) {
            result = VCALL(result, simplify, &wasTrivial);
            cycles++;
        }

        if (cycles > 1) {
            TEXP("If we simplify it, we get:\\par\n"
                 "$ ");
            result->writeTex(logFile);
            TEXP(" $\\par\n");
        }

        return result;
    }


    ExprNode *ExprNode::VMIN(BinOp_Add, diff)(char by, FILE *logFile) {
        TEXP("Thankfully, the derivative is additive.\\par\n"
             "$ \\frac{d}{d%c} (", by);
        this->writeTex(logFile);
        TEXP(") = (");
        left->writeTex(logFile);
        TEXP(")' + (");
        right->writeTex(logFile);
        TEXP(")' $\\par\n");

        return ADD_(DIFF_(left), DIFF_(right));
    }

    ExprNode *ExprNode::VMIN(BinOp_Sub, diff)(char by, FILE *logFile) {
        TEXP("Thankfully, the derivative is additive (even in subtraction).\\par\n"
             "$ \\frac{d}{d%c} (", by);
        this->writeTex(logFile);
        TEXP(") = (");
        left->writeTex(logFile);
        TEXP(")' - (");
        right->writeTex(logFile);
        TEXP(")' $\\par\n");

        return SUB_(DIFF_(left), DIFF_(right));
    }

    ExprNode *ExprNode::VMIN(BinOp_Mul, diff)(char by, FILE *logFile) {
        TEXP("Multiplication is a bit difficult, but we can still manage it.\\par\n"
             "$ \\frac{d (f \\cdot g)}{d%c} = f' \\cdot g + f \\cdot g' $\\par\n"
             "$ f(x) = ", by);
        left->writeTex(logFile);
        TEXP(" $\\par\n$ g(x) = ");
        right->writeTex(logFile);
        TEXP(" $\\par\n");

        return ADD_(MUL_(DIFF_(left), COPY_(right)), MUL_(COPY_(left), DIFF_(right)));
    }

    ExprNode *ExprNode::VMIN(BinOp_Div, diff)(char by, FILE *logFile) {
        TEXP("Division is troubling, but I guess we have no choice...\\par\n"
             "$ \\frac{d}{d%c} \\frac{f}{g} = \\frac{f' \\cdot g - f \\cdot g'}{g^2} $\\par\n"
             "$ f(x) = ", by);
        left->writeTex(logFile);
        TEXP(" $\\par\n$ g(x) = ");
        right->writeTex(logFile);
        TEXP(" $\\par\n");

        return DIV_(SUB_(MUL_(DIFF_(left), COPY_(right)), MUL_(COPY_(left), DIFF_(right))), POW_(COPY_(right), CONST_(2)));
    }

    ExprNode *ExprNode::VMIN(BinOp_Pow, diff)(char by, FILE *logFile) {
        if (VCALL(right, isConstBy, by)) {
            TEXP("Polynomial differentiation is a piece of cake.\\par\n"
                 "$ \\frac{d}{d%c} f^{a} = a \\cdot f^{a - 1} \\cdot f' $\\par\n"
                 "$ f(x) = ", by);
            left->writeTex(logFile);
            TEXP(" $\\par\n$ a = ");
            right->writeTex(logFile);
            TEXP(" $\\par\n");

            return MUL_(MUL_(COPY_(right), POW_(COPY_(left), SUB_(COPY_(right), CONST_(1)))), DIFF_(left));
        } else if (VCALL(left, isConstBy, by)) {
            TEXP("Exponential differentiation is not as scary as it looks.\n\n"
                 "$ \\frac{d}{d%c} a^f = \\ln{a} \\cdot a^f \\cdot f' $\\par\n"
                 "$ f(x) = ", by);
            left->writeTex(logFile);
            TEXP(" $\\par\n$ a = ");
            right->writeTex(logFile);
            TEXP(" $\\par\n");

            return MUL_(MUL_(LN_(COPY_(left)), COPY_(this)), DIFF_(right));
        } else {
            /*TEXP("Oops... Now this is awkward. I have no idea how to handle exponential-polynomial hybrid differentiation.\n"
                 "Just write this on my gravestone..."
                 "$$ \\frac{d}{d%c} ", by);
            writeTex(logFile);
            TEXP(" = ? $$\n\n");

            return VAR_('Ú');*/

            TEXP("I kind of had to look this up, but eventually I figured out that the\n"
                 "derivative for this kind of stuff is called \"generalized power rule\":\\par\n"
                 "$ (f^g)' = f^g\\cdot (f'\\cdot\\frac{g}{f}+g'\\cdot\\ln{f}) $\\par\n");
            TEXP("$ f(x) = ");
            left->writeTex(logFile);
            TEXP(" $\\par\n$ g(x) = ");
            right->writeTex(logFile);
            TEXP(" $\\par\n");

            return MUL_(COPY_(this), ADD_(MUL_(DIFF_(left), DIV_(COPY_(left), COPY_(right))), MUL_(DIFF_(right), LN_(left))));
        }
    }

    ExprNode *ExprNode::VMIN(BinOp, simplify)(bool *wasTrivial) {
        int cnt = 0;

        *wasTrivial = false;
        while (!*wasTrivial) {
            left = VCALL(left, simplify, wasTrivial);
            cnt++;
        }


        *wasTrivial = false;
        while (!*wasTrivial) {
            right = VCALL(right, simplify, wasTrivial);
            cnt++;
        }

        *wasTrivial = false;
        ExprNode *result = (this->*binOpSimplifiers[binOp])(wasTrivial);

        *wasTrivial &= cnt == 2;

        return result;
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

    static void writeTexParentheses_(FILE *ofile, ExprNode *node, bool doParetheses) {
        if (doParetheses) {
            fprintf(ofile, "(");
        }

        node->writeTex(ofile);

        if (doParetheses) {
            fprintf(ofile, ")");
        }
    }

    static void writeTexFrac_(FILE *ofile, ExprNode *left, ExprNode *right, bool lBrackets, bool rBrackets) {
        const unsigned THRESHOLD = 10;

        unsigned lCompl = VCALL(left,  getComplexity);
        unsigned rCompl = VCALL(right, getComplexity);

        if (lCompl >= THRESHOLD || rCompl >= THRESHOLD) {
            writeTexParentheses_(ofile, left,  lBrackets);
            fprintf(ofile, " \\div ");
            writeTexParentheses_(ofile, right, rBrackets);
        } else {
            fprintf(ofile, "\\frac{");
            writeTexParentheses_(ofile, left,  false);
            fprintf(ofile, "}{");
            writeTexParentheses_(ofile, right, false);
            fprintf(ofile, "}");
        }
    }

    void ExprNode::VMIN(BinOp, writeTex)(FILE *ofile) {
        Priority_e  prio = VCALL(this,  getPriority);
        Priority_e lprio = VCALL(left,  getPriority);
        //printf("\n>> %p : %d %p %p\n", this, binOp, left, right);
        Priority_e rprio = VCALL(right, getPriority);

        bool lBrackets = lprio < prio || (lprio == prio && binOp == BinOp_Pow);
        bool rBrackets = rprio < prio || (lprio == prio && binOp == BinOp_Sub || binOp == BinOp_Div);

        #define LEFT_   writeTexParentheses_(ofile, left,  lBrackets);
        #define RIGHT_  writeTexParentheses_(ofile, right, rBrackets);

        switch (binOp) {
        case BinOp_Add:
        case BinOp_Sub:
            LEFT_
            fprintf(ofile, " %s ", binOpStrings[binOp]);
            RIGHT_
            break;
        case BinOp_Mul:
            LEFT_
            fprintf(ofile, " \\cdot ");
            RIGHT_
            break;
        case BinOp_Div:
            writeTexFrac_(ofile, left, right, lBrackets, rBrackets);
            break;
        case BinOp_Pow:
            fprintf(ofile, "{");
            LEFT_
            fprintf(ofile, "}^{");
            rBrackets = false;
            RIGHT_
            fprintf(ofile, "}");
            break;
        default:
            assert(false);
            break;
        }

        #undef LEFT_
        #undef RIGHT_
    }

    Priority_e ExprNode::VMIN(BinOp, getPriority)() {
        switch (binOp) {
        case BinOp_Add:
        case BinOp_Sub:
            return Priority_Add;
        case BinOp_Mul:
        case BinOp_Div:
            return Priority_Mul;
        case BinOp_Pow:
            return Priority_Pow;
        default:
            assert(false);
            abort();
        }
    }

    bool ExprNode::VMIN(BinOp, isConstBy)(char by) {
        return VCALL(left, isConstBy, by) && VCALL(right, isConstBy, by);
    }

    unsigned ExprNode::VMIN(BinOp, getComplexity)() {
        return 1 + VCALL(left, getComplexity) + VCALL(right, getComplexity);
    }

    //--------------------------------------------------------------------------------

    VTYPE_DEF(BinOp, ExprNode) = {
        ExprNode::VMIN(BinOp, dtor),
        ExprNode::VMIN(BinOp, dump),
        ExprNode::VMIN(BinOp, diff),
        ExprNode::VMIN(BinOp, copy),
        ExprNode::VMIN(BinOp, simplify),
        ExprNode::VMIN(BinOp, writeTex),
        ExprNode::VMIN(BinOp, getPriority),
        ExprNode::VMIN(BinOp, isConstBy),
        ExprNode::VMIN(BinOp, getComplexity),
    };
}


#include "expr.h"

#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <cmath>


namespace SymbolDiff {

    ExprNode *ExprNode::ctorUnOp(UnOp_e new_unOp, ExprNode *new_child) {
        VSETTYPE(this, UnOp);

        REQUIRE(new_child);

        unOp = new_unOp;
        child = new_child;

        return this;
    }


    void ExprNode::VMIN(UnOp, dump)() {
        printf("(%s ", unOpStrings[unOp]);
        VCALL(child, dump);
        printf(")");
    }

    void ExprNode::VMIN(UnOp, dtor)() {
        if (child) {
            child->destroy();
        }
    }

    #include "expr_dsl_def.h"

    ExprNode *ExprNode::VMIN(UnOp, diff)(char by, FILE *logFile) {
        ExprNode *result = (this->*unOpDifferentiators[unOp])(by, logFile);

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


    ExprNode *ExprNode::VMIN(UnOp_Neg, diff)(char by, FILE *logFile) {
        TEXP("I don't think this needs clarification.\\par\n"
             "$ \\frac{d}{d%c} (", by);
        writeTex(logFile);
        TEXP(") = -(");
        child->writeTex(logFile);
        TEXP(")'$\\par\n");

        return NEG_(DIFF_(child));
    }

    ExprNode *ExprNode::VMIN(UnOp_Sin, diff)(char by, FILE *logFile) {
        TEXP("Sine turns into cosine.\\par\n"
             "$ \\frac{d}{d%c} ", by);
        writeTex(logFile);
        TEXP(" = \\cos (");
        child->writeTex(logFile);
        TEXP(") \\cdot (");
        child->writeTex(logFile);
        TEXP(")' $\\par\n");

        return MUL_(COS_(COPY_(child)), DIFF_(child));
    }

    ExprNode *ExprNode::VMIN(UnOp_Cos, diff)(char by, FILE *logFile) {
        TEXP("Cosine turns into negative sine.\\par\n"
             "$ \\frac{d}{d%c} ", by);
        writeTex(logFile);
        TEXP(" = -\\sin (");
        child->writeTex(logFile);
        TEXP(") \\cdot (");
        child->writeTex(logFile);
        TEXP(")' $\\par\n");

        return NEG_(MUL_(SIN_(COPY_(child)), DIFF_(child)));
    }

    ExprNode *ExprNode::VMIN(UnOp_Ln, diff)(char by, FILE *logFile) {
        TEXP("Natural logarithm is a beautiful function.\\par\n"
             "$ \\frac{d}{d%c} ", by);
        writeTex(logFile);
        TEXP(" = \\frac{(");
        child->writeTex(logFile);
        TEXP(")'}{");
        child->writeTex(logFile);
        TEXP("} $\\par\n");

        return DIV_(DIFF_(child), COPY_(child));
    }


    ExprNode *ExprNode::VMIN(UnOp, simplify)(bool *wasTrivial) {
        int cnt = 0;

        *wasTrivial = false;
        while (!*wasTrivial) {
            child = VCALL(child, simplify, wasTrivial);
            cnt++;
        }

        *wasTrivial = false;
        ExprNode *result = (this->*unOpSimplifiers[unOp])(wasTrivial);

        *wasTrivial &= cnt == 1;

        return result;
    }


    ExprNode *ExprNode::VMIN(UnOp_Neg, simplify)(bool *wasTrivial) {
        if (VISINST(child, UnOp) && child->unOp == UnOp_Neg) {
            ExprNode *tmp = child->child;

            child->child = nullptr;
            destroy();

            return tmp;
        }

        if (VISINST(child, Const)) {
            ExprNode *tmp = child;

            child = nullptr;
            destroy();

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

    ExprNode *ExprNode::VMIN(UnOp_Ln, simplify)(bool *wasTrivial) {
        *wasTrivial = true;

        return this;
    }

    #include "expr_dsl_undef.h"


    ExprNode *ExprNode::VMIN(UnOp, copy)() {
        return CCRN(ExprNode, ctorUnOp, unOp, VCALL(child, copy));
    }

    void ExprNode::VMIN(UnOp, writeTex)(FILE *ofile) {
        Priority_e  prio = VCALL(this,  getPriority);
        Priority_e cprio = VCALL(child, getPriority);

        bool cBrackets = cprio < prio;

        #define CHILD_                          \
            if (cBrackets) {                    \
                fprintf(ofile, "(");      \
            }                                   \
                                                \
            child->writeTex(ofile);      \
                                                \
            if (cBrackets) {                    \
                fprintf(ofile, ")");     \
            }

        switch (unOp) {
        case UnOp_Neg:
            fprintf(ofile, "- ");
            CHILD_;
            break;
        case UnOp_Sin:
        case UnOp_Cos:
        case UnOp_Ln:   // TODO: ?
            fprintf(ofile, "\\%s ", unOpStrings[unOp]);
            CHILD_;
            break;
        default:
            assert(false);
            break;
        }
    }

    Priority_e ExprNode::VMIN(UnOp, getPriority)() {
        switch (unOp) {
        case UnOp_Neg:
            return Priority_Neg;
        case UnOp_Sin:
        case UnOp_Cos:
        case UnOp_Ln:   // TODO: ?
            return Priority_Ufunc;
        default:
            assert(false);
            abort();
        }
    }

    bool ExprNode::VMIN(UnOp, isConstBy)(char by) {
        return VCALL(child, isConstBy, by);
    }

    unsigned ExprNode::VMIN(UnOp, getComplexity)() {
        return 1 + VCALL(child, getComplexity);
    }

    //--------------------------------------------------------------------------------

    VTYPE_DEF(UnOp, ExprNode) = {
        ExprNode::VMIN(UnOp, dtor),
        ExprNode::VMIN(UnOp, dump),
        ExprNode::VMIN(UnOp, diff),
        ExprNode::VMIN(UnOp, copy),
        ExprNode::VMIN(UnOp, simplify),
        ExprNode::VMIN(UnOp, writeTex),
        ExprNode::VMIN(UnOp, getPriority),
        ExprNode::VMIN(UnOp, isConstBy),
        ExprNode::VMIN(UnOp, getComplexity),
    };
}



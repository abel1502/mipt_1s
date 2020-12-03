#include "expr.h"

#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <cmath>


namespace SymbolDiff {

    ExprNode *ExprNode::ctorUnOp(UnOp_e new_unOp, ExprNode *new_child) {
        VSETTYPE(this, UnOp);

        assert(new_child);

        unOp = new_unOp;
        child = new_child;

        return this;
    }


    void ExprNode::VMIN(UnOp, dump)() {
        printf("(%s (", unOpStrings[unOp]);
        VCALL(child, dump);
        printf(")");
    }

    void ExprNode::VMIN(UnOp, dtor)() {
        if (child) {
            child->destroy();
        }
    }


    #include "expr_dsl_def.h"

    ExprNode *ExprNode::VMIN(UnOp, diff)(char by) {
        return (this->*unOpDifferentiators[unOp])(by);
    }


    ExprNode *ExprNode::VMIN(UnOp_Neg, diff)(char by) {
        return NEG_(DIFF_(child));
    }

    ExprNode *ExprNode::VMIN(UnOp_Sin, diff)(char by) {
        return MUL_(COS_(COPY_(child)), DIFF_(child));
    }

    ExprNode *ExprNode::VMIN(UnOp_Cos, diff)(char by) {
        return NEG_(MUL_(SIN_(COPY_(child)), DIFF_(child)));
    }


    ExprNode *ExprNode::VMIN(UnOp, simplify)(bool *wasTrivial) {
        *wasTrivial = false;
        while (!*wasTrivial)
            child = VCALL(child, simplify, wasTrivial);

        return (this->*unOpSimplifiers[unOp])(wasTrivial);
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

    #include "expr_dsl_undef.h"


    ExprNode *ExprNode::VMIN(UnOp, copy)() {
        return ExprNode::create()->ctorUnOp(unOp, VCALL(child, copy));
    }


    VTYPE_DEF(UnOp, ExprNode) = {
        ExprNode::VMIN(UnOp, dtor),
        ExprNode::VMIN(UnOp, dump),
        ExprNode::VMIN(UnOp, diff),
        ExprNode::VMIN(UnOp, copy),
        ExprNode::VMIN(UnOp, simplify),
    };
}



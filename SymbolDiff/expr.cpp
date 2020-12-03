#include "expr.h"

#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <cmath>
#include <cctype>
#include <cstring>


namespace SymbolDiff {

    ExprNode *ExprNode::ctor() {
        return this;  // TODO: Zero-fill?
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

    ExprNode *ExprNode::read(FILE *ifile) {
        Parser p;
        p.ctor();
        p.feed(ifile);

        ExprNode *result = read(&p);

        p.dtor();

        return result;
    }

    ExprNode *ExprNode::read(const char *src) {
        Parser p;
        p.ctor();
        p.feed(src);

        ExprNode *result = read(&p);

        p.dtor();

        return result;
    }

    ExprNode *ExprNode::read(Parser *parser) {
        ExprNode *result = ExprNode::create();

        if (parser->parse(result)) {
            result->destroy();

            result = nullptr;
        }

        return result;
    }

    void ExprNode::dtor() {
        //printf("[%p] %p %p\n", this, vtable_, vtable_ ? (void *)vtable_->dtor : (void *)-1);

        if (vtable_) {
            /*if (VISINST(this, BinOp)) {
                printf("> BinOp %p %s %p\n", left, binOpStrings[binOp], right);
            } else if (VISINST(this, UnOp)) {
                printf("> UnOp %s %p\n", unOpStrings[unOp], child);
            } else if (VISINST(this, Const)) {
                printf("> Const %lld\n", value);
            } else if (VISINST(this, Var)) {
                printf("> Var %c\n", varName);
            } else {
                printf("> ???\n", value);
            }*/

            VCALL(this, dtor);
        }
    }

    //--------------------------------------------------------------------------------

    void ExprNode::VMIN(Const, dump)() {
        printf("(%lld)", value);
    }

    void ExprNode::VMIN(Var, dump)() {
        printf("(%c)", varName);
    }

    void ExprNode::VMIN(Leaf, dtor)() {}


    #include "expr_dsl_def.h"

    ExprNode *ExprNode::VMIN(Const, diff)(char __attribute__((unused)) by, FILE *logFile) {
        return CONST_(0);
    }

    ExprNode *ExprNode::VMIN(Var, diff)(char by, FILE *logFile) {
        if (varName == by)
            return CONST_(1);

        return CONST_(0);
    }

    ExprNode *ExprNode::VMIN(Leaf, simplify)(bool *wasTrivial) {
        *wasTrivial = true;

        return this;
    }

    ExprNode *ExprNode::VMIN(Const, copy)() {
        return ExprNode::create()->ctorConst(value);
    }

    ExprNode *ExprNode::VMIN(Var, copy)() {
        return ExprNode::create()->ctorVar(varName);
    }


    void ExprNode::VMIN(Const, writeTex)(FILE *ofile) {
        fprintf(ofile, " %lld ", value);
    }

    void ExprNode::VMIN(Var, writeTex)(FILE *ofile) {
        fprintf(ofile, " %c ", varName);
    }


    Priority_e ExprNode::VMIN(Const, getPriority)() {
        if (value < 0)
            return Priority_Neg;

        return Priority_Imm;
    }

    Priority_e ExprNode::VMIN(Var, getPriority)() {
        return Priority_Imm;
    }

    bool ExprNode::VMIN(Const, isConstBy)(char __attribute__((unused)) by) {
        return true;
    }

    bool ExprNode::VMIN(Var, isConstBy)(char by) {
        return by != varName;
    }


    VTYPE_DEF(Const, ExprNode) = {
        ExprNode::VMIN(Leaf, dtor),
        ExprNode::VMIN(Const, dump),
        ExprNode::VMIN(Const, diff),
        ExprNode::VMIN(Const, copy),
        ExprNode::VMIN(Leaf, simplify),
        ExprNode::VMIN(Const, writeTex),
        ExprNode::VMIN(Const, getPriority),
        ExprNode::VMIN(Const, isConstBy),
    };

    VTYPE_DEF(Var, ExprNode) = {
        ExprNode::VMIN(Leaf, dtor),
        ExprNode::VMIN(Var, dump),
        ExprNode::VMIN(Var, diff),
        ExprNode::VMIN(Var, copy),
        ExprNode::VMIN(Leaf, simplify),
        ExprNode::VMIN(Var, writeTex),
        ExprNode::VMIN(Var, getPriority),
        ExprNode::VMIN(Var, isConstBy),
    };

    //--------------------------------------------------------------------------------

    ExprTree *ExprTree::ctor() {
        root = nullptr;

        return this;
    }

    void ExprTree::dtor() {
        if (root)
            root->destroy();

        root = nullptr;
    }

    bool ExprTree::read(FILE *ifile) {
        root = ExprNode::read(ifile);

        return (bool)root;  // Explicit cast for readability
    }

    bool ExprTree::read(const char *src) {
        root = ExprNode::read(src);

        return (bool)root;  // Same deal
    }

    void ExprTree::writeTex(FILE *ofile) {
        assert(root);

        fprintf(ofile, "$$ ");
        VCALL(root, writeTex, ofile);
        fprintf(ofile, " $$\n");
    }

    void ExprTree::dump() {
        assert(root);

        VCALL(root, dump);
        printf("\n");
    }

    void ExprTree::simplify() {
        assert(root);

        bool wasTrivial = false;
        while (!wasTrivial)
            root = VCALL(root, simplify, &wasTrivial);
    }

    ExprTree *ExprTree::diff(char by) {
        assert(root);

        ExprTree *newTree = ExprTree::create();

        newTree->root = VCALL(root, diff, by, nullptr);

        return newTree;
    }

}


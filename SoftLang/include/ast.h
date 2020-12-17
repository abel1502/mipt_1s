#ifndef AST_H
#define AST_H

#include "general.h"
#include "parser.h"
#include "vector.h"
#include "dict.h"
#include "vtable.h"

#include <cstdio>


namespace SoftLang {

    class TypeSpec {
    public:

        enum Type_e {
            T_VOID,
            T_DBL,
            T_INT4,
            T_INT8
        };

        FACTORIES(TypeSpec)

        bool ctor();

        bool ctor(Type_e type);

        void dtor();

    private:

        //

    };

    class Var {
    public:

        FACTORIES(Var)

        bool ctor();

        void dtor();

    private:

        //

    };

    class Scope {
    public:

        FACTORIES(Scope)

        bool ctor();

        void dtor();

    private:

        //

    };

    class Expression {  // Abstract
    public:

        enum AsgnMode_e {
            AM_EQ,
            AM_ADDEQ,
            AM_SUBEQ,
            AM_MULEQ,
            AM_DIVEQ,
            AM_MODEQ
        };

        enum Op_e {
            OP_EQ,
            OP_NEQ,
            OP_GEQ,
            OP_LULEQ,
            OP_LT,
            OP_GT,

            OP_ADD,
            OP_SUB,
            OP_MUL,
            OP_DIV,
            OP_MOD
        };

        FACTORIES(Expression)

        bool ctor();

        void dtor();

    private:

        //

    };

    class Statement {  // Abstract
    public:

        FACTORIES(Statement)

        bool ctor();

        bool ctorCompound();

        bool ctorReturn();

        bool ctorLoop();

        bool ctorCond();

        bool ctorVardecl();

        bool ctorExpr();

        bool ctorEmpty();

        void dtor();

        bool makeCode(Code **code);

        bool makeAltCode(Code **altCode);

    private:

        //

    };

    class Code {
    public:

        FACTORIES(Code)

        bool ctor();

        void dtor();

        bool makeStatement(Statement **stmt);

    private:

        Scope scope;

        Vector<Statement> stmts;

    };

    class Function {
    public:

        FACTORIES(Function)

        bool ctor();

        bool ctor(const TypeSpec &rtype, const Token *name);

        void dtor();

        /// Same as makeFunction
        bool makeArg(Var **arg);

        /// -"-
        bool makeStmt(Statement **stmt);

        /// This one will always return &code
        bool makeCode(Code **code);

        bool write(FILE *ofile);

    private:

        Vector<Var> args;
        Code code;
        TypeSpec rtype;
        const Token *name;

    };

    class Program {
    public:

        FACTORIES(Program)

        bool ctor();

        void dtor();

        /// Allocates space for a new Function object and stores a pointer to it into `dest`
        /// (Space is allocated in a Vector, which prevents memory fragmentation, but
        ///  requires that the resulting pointer isn't stored, since it may break upon reallocation)
        bool makeFunction(Function **dest);

        bool write(FILE *ofile);

    private:

        Vector<Function> functions;
        // unsigned mainInd; // Not sure if I truly need it

    };

}


#endif // AST_H
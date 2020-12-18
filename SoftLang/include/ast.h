#ifndef AST_H
#define AST_H

#include "general.h"
#include "parser.h"
#include "vector.h"
#include "dict.h"
#include "vtable.h"

#include <cstdio>
#include <cstdint>


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

        bool ctor(Type_e new_type);

        void dtor();

    private:

        Type_e type;

    };

    class Var {
    public:

        FACTORIES(Var)

        bool ctor();

        bool ctor(TypeSpec new_ts, const Token *new_name);

        void dtor();

    private:

        TypeSpec ts;

        const Token *name;

    };

    class Program;

    class Scope {
    public:

        FACTORIES(Scope)

        bool ctor();

        void dtor();

        // TODO

    private:

        StrDict<unsigned> vars;
        Vector<uint32_t> varOffsets;
        const Program *prog;

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
            OP_LEQ,
            OP_LT,
            OP_GT,

            OP_ADD,
            OP_SUB,
            OP_MUL,
            OP_DIV,
            OP_MOD
        };

        VTABLE_STRUCT {
            VDECL(Expression, void, dtor);
        };

        VTABLE_FIELD

        #define DEF_TYPE(NAME)  VTYPE_FIELD(NAME);
        #include "exprtypes.dsl.h"
        #undef DEF_TYPE


        FACTORIES(Expression)

        bool ctor();

        bool ctorVoid();

        bool ctorAsgn();

        bool ctorPolyOp();

        bool ctorNeg();

        bool ctorCast(TypeSpec ts);

        bool ctorNum(const Token *new_num);

        bool ctorVarRef(const Token *new_name);

        bool ctorFuncCall(const Token *new_name);

        void dtor();

        bool makeChild(Expression **child);

        void setAsgnMode(AsgnMode_e mode);

        bool setOp(unsigned ind, Op_e op);

        void simplifySingleChild();

        #define DEF_TYPE(NAME) \
            bool is##NAME() const;
        #include "exprtypes.dsl.h"
        #undef DEF_TYPE

    private:

        union {
            // Void

            // Asgn
            AsgnMode_e am;

            // PolyOp
            CompactVector<Op_e> ops;

            // Neg

            // Cast
            TypeSpec cast;

            // Num
            const Token *num;

            // VarRef, FuncCall
            const Token *name;
        };

        Vector<Expression> children;

        #define DEF_TYPE(NAME) \
            void VMIN(NAME, dtor)();
        #include "exprtypes.dsl.h"
        #undef DEF_TYPE

    };

    class Statement;

    class Code {
    public:

        FACTORIES(Code)

        bool ctor();

        void dtor();

        bool makeStatement(Statement **stmt);

        void simplifyLastEmpty();

    private:

        Scope scope;

        Vector<Statement> stmts;

    };

    class Statement {  // Abstract
    public:

        VTABLE_STRUCT {
            VDECL(Statement, void, dtor);
        };

        VTABLE_FIELD

        #define DEF_TYPE(NAME)  VTYPE_FIELD(NAME);
        #include "stmttypes.dsl.h"
        #undef DEF_TYPE

        FACTORIES(Statement)

        bool ctor();

        bool ctorCompound();

        bool ctorReturn();

        bool ctorLoop();

        bool ctorCond();

        bool ctorVarDecl();

        bool ctorExpr();

        bool ctorEmpty();

        void dtor();

        bool makeCode(Code **out_code);

        bool makeAltCode(Code **out_altCode);

        bool makeExpr(Expression **out_expr);

        bool makeVar(Var **out_var);

        #define DEF_TYPE(NAME) \
            bool is##NAME() const;
        #include "stmttypes.dsl.h"
        #undef DEF_TYPE

    private:

        Expression expr;

        union {
            struct {
                Code code;
                Code altCode;
            };

            Var var;
        };

        #define DEF_TYPE(NAME) \
            void VMIN(NAME, dtor)();
        #include "stmttypes.dsl.h"
        #undef DEF_TYPE

    };

    class Function {
    public:

        FACTORIES(Function)

        bool ctor();

        bool ctor(TypeSpec new_rtype, const Token *new_name);

        void dtor();

        /// Same as makeFunction
        bool makeArg(Var **arg);

        /// This one will always return &code
        bool makeCode(Code **code);

        bool compile(FILE *ofile);

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

        bool compile(FILE *ofile);

    private:

        Vector<Function> functions;
        // unsigned mainInd; // Not sure if I truly need it

    };

}


#endif // AST_H

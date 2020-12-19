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

    struct TypeSpec {

        enum Type_e {
            T_VOID,
            T_DBL,
            T_INT4,
            T_INT8,
        };

        static const unsigned TYPES_COUNT = 4;  // TODO: Needs manual adjustments

        typedef unsigned Mask;

        static constexpr Mask NoneMask = 0;
        static constexpr Mask AllMask  = (1 << TYPES_COUNT) - 1;
        static constexpr Mask VoidMask = 1 << T_VOID;
        static constexpr Mask DblMask  = 1 << T_DBL;
        static constexpr Mask Int4Mask = 1 << T_INT4;
        static constexpr Mask Int8Mask = 1 << T_INT8;


        Type_e type;


        FACTORIES(TypeSpec)

        bool ctor();

        bool ctor(Type_e new_type);

        bool ctor(Mask mask);

        void dtor();

        bool compile(FILE *ofile);

        uint32_t getSize() const;

        Mask getMask() const;

        bool fitsMask(Mask mask) const;

        static constexpr bool isMaskUnambiguous(Mask mask);

    };

    //================================================================================

    class Scope;

    class Var {
    public:

        FACTORIES(Var)

        bool ctor();

        bool ctor(TypeSpec new_ts, const Token *new_name);

        void dtor();

        bool compile(FILE *ofile, const Scope *scope);

        const TypeSpec getType() const;

        const Token *getName() const;

    private:

        TypeSpec ts;
        const Token *name;

    };

    //================================================================================

    class Program;

    class Scope {
    public:

        struct VarInfo {
            uint32_t offset;
            const Var *var;
        };


        FACTORIES(Scope)

        bool ctor();

        void dtor();

        VarInfo getInfo(const Var *var) const;

        VarInfo getInfo(const Token *name) const;

        bool hasVar(const Var *var) const;

        bool hasVar(const Token *name) const;

        bool addVar(const Var *var);

        void setParent(const Scope *new_parent);

        uint32_t getFrameSize() const;

    private:

        const Scope *parent;
        uint32_t curOffset;
        NameDict<VarInfo> vars;

    };

    //================================================================================

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
            VDECL(Expression, TypeSpec::Mask, deduceType, Scope *scope, const Program *prog);
            VDECL(Expression, bool, compile, FILE *ofile, Scope *scope, const Program *prog);
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

        TypeSpec::Mask deduceType(TypeSpec::Mask mask, Scope *scope, const Program *prog);

        bool compile(FILE *ofile, Scope *scope, const Program *prog);

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

        TypeSpec::Mask typeMask;

        Vector<Expression> children;

        #define DEF_TYPE(NAME) \
            void VMIN(NAME, dtor)();
        #include "exprtypes.dsl.h"
        #undef DEF_TYPE

        #define DEF_TYPE(NAME) \
            TypeSpec::Mask VMIN(NAME, deduceType)(Scope *scope, const Program *prog);
        #include "exprtypes.dsl.h"
        #undef DEF_TYPE

        #define DEF_TYPE(NAME) \
            bool VMIN(NAME, compile)(FILE *ofile, Scope *scope, const Program *prog);
        #include "exprtypes.dsl.h"
        #undef DEF_TYPE

    };

    //================================================================================

    class Statement;
    class Function;

    class Code {
    public:

        FACTORIES(Code)

        bool ctor();

        void dtor();

        bool makeStatement(Statement **stmt);

        void simplifyLastEmpty();

        bool compile(FILE *ofile, TypeSpec rtype, const Program *prog);

        Scope *getScope();

    private:

        Scope scope;
        Vector<Statement> stmts;

    };

    //================================================================================

    class Statement {  // Abstract
    public:

        VTABLE_STRUCT {
            VDECL(Statement, void, dtor);
            VDECL(Statement, bool, compile, FILE *ofile, Scope *scope, TypeSpec rtype, const Program *prog);
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

        bool compile(FILE *ofile, Scope *scope, TypeSpec rtype, const Program *prog);

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

        #define DEF_TYPE(NAME) \
            bool VMIN(NAME, compile)(FILE *ofile, Scope *scope, TypeSpec rtype, const Program *prog);
        #include "stmttypes.dsl.h"
        #undef DEF_TYPE

    };

    //================================================================================

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

        /// Register all args into the current scope
        bool registerArgs();

        bool compile(FILE *ofile, const Program *prog);

        bool isMain() const;

        TypeSpec getRtype() const;

        const Token *getName() const;

    private:

        Vector<Var> args;
        Code code;
        TypeSpec rtype;
        const Token *name;

    };

    //================================================================================

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

        const Function *getFunction(const Token *name) const;

    private:

        Vector<Function> functions;
        // unsigned mainInd; // Not sure if I truly need it

    };

}


#endif // AST_H

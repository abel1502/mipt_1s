#include "ast.h"


namespace SoftLang {

    bool TypeSpec::ctor() {
        type = TypeSpec::T_VOID;

        return false;
    }

    bool TypeSpec::ctor(Type_e new_type) {
        type = new_type;

        return false;
    }

    void TypeSpec::dtor() {
        type = TypeSpec::T_VOID;
    }

    bool TypeSpec::compile(FILE *ofile) {
        switch (type) {
        case TypeSpec::T_DBL:
            fprintf(ofile, "df:");
            break;

        case TypeSpec::T_INT4:
            fprintf(ofile, "dwl:");
            break;

        case TypeSpec::T_INT8:
            fprintf(ofile, "qwl:");
            break;

        case TypeSpec::T_VOID:
            ERR("Void is not a valid type");
            // FALLTHROUGH
        default:
            return true;
        }

        return false;
    }

    uint32_t TypeSpec::getSize() const {
        switch (type) {
        case TypeSpec::T_DBL:
            return 8;

        case TypeSpec::T_INT4:
            return 4;

        case TypeSpec::T_INT8:
            return 8;

        case TypeSpec::T_VOID:
            return 0;  // TODO: Fail?

        default:
            break;
        }

        assert(false);
        return -1;
    }

    TypeSpec::Type_e TypeSpec::getType() const {
        return type;
    }

    bool Var::ctor() {
        ts = {};
        name = nullptr;

        return false;
    }

    bool Var::ctor(TypeSpec new_ts, const Token* new_name) {
        ts = new_ts;
        name = new_name;

        return false;
    }

    void Var::dtor() {
        ts = {};
        name = {};
    }

    bool Var::compile(const Scope *scope, FILE *ofile) {
        if (!scope->hasVar(this)) {
            ERR("Unknown variable \"%.*s\"", name->getLength(), name->getStr());

            return true;
        }

        uint32_t offset = scope->getOffset(this);

        TRY_B(ts.compile(ofile));
        fprintf(ofile, "[rzx+%u]", offset);

        return false;
    }

    const TypeSpec Var::getType() const {
        return ts;
    }

    const Token *Var::getName() const {
        return name;
    }

    bool Scope::ctor() {
        TRY_B(vars.ctor());
        curOffset = 0;

        return false;
    }

    void Scope::dtor() {
        vars.dtor();
    }

    uint32_t Scope::getOffset(const Var *var) const {
        if (!hasVar(var))
            return -1;

        return vars.get(var->getName());
    }

    bool Scope::hasVar(const Var *var) const {
        return vars.contains(var->getName());
    }

    bool Scope::addVar(const Var *var) {
        if (hasVar(var)) {
            ERR("Redeclaration of variable \"%.*s\"", var->getName()->getLength(), var->getName()->getStr());

            return true;
        }

        uint32_t offset = curOffset;

        curOffset += var->getType().getSize();

        TRY_B(vars.set(var->getName(), offset));

        return false;
    }

    bool Expression::ctor() {
        // TODO: zero-fill?
        return false;
    }

    bool Expression::ctorVoid() {
        VSETTYPE(this, Void);

        return false;
    }

    bool Expression::ctorAsgn() {
        VSETTYPE(this, Asgn);

        am = Expression::AM_EQ;
        TRY_B(children.ctor());

        return false;
    }

    bool Expression::ctorPolyOp() {
        VSETTYPE(this, PolyOp);

        TRY_B(ops.ctor());
        TRY_B(children.ctor());

        return false;
    }

    bool Expression::ctorNeg() {
        VSETTYPE(this, Neg);

        TRY_B(children.ctor());

        return false;
    }

    bool Expression::ctorCast(TypeSpec ts) {
        VSETTYPE(this, Cast);

        cast = ts;
        TRY_B(children.ctor());

        return false;
    }

    bool Expression::ctorNum(const Token *new_num) {
        VSETTYPE(this, Num);

        assert(new_num->isNum());
        num = new_num;

        return false;
    }

    bool Expression::ctorVarRef(const Token *new_name) {
        VSETTYPE(this, VarRef);

        assert(new_name->isName());
        name = new_name;

        return false;
    }

    bool Expression::ctorFuncCall(const Token* new_name) {
        VSETTYPE(this, FuncCall);

        assert(new_name->isName());
        name = new_name;
        TRY_B(children.ctor());

        return false;
    }

    void Expression::dtor() {
        children.dtor();
        VCALL(this, dtor);
    }

    bool Expression::makeChild(Expression** child) {
        TRY_B(children.append({}));

        *child = &children[-1];

        return false;
    }

    void Expression::setAsgnMode(AsgnMode_e mode) {
        assert(isAsgn());

        am = mode;
    }

    bool Expression::setOp(unsigned ind, Op_e op) {
        assert(isPolyOp());

        if (ind == ops.getSize()) {
            TRY_B(ops.append(op));
        }

        assert(ind < ops.getSize());
        ops[ind] = op;

        return false;
    }

    void Expression::simplifySingleChild() {
        if (!isPolyOp() || children.getSize() != 1)
            return;

        // TODO: Actually, this wouldn't destroy stuff properly
        memcpy(this, &children[0], sizeof(Expression));
    }

    #define DEF_TYPE(NAME) \
        bool Expression::is##NAME() const { \
            return VISINST(this, NAME); \
        }
    #include "exprtypes.dsl.h"
    #undef DEF_TYPE

    void Expression::VMIN(Void, dtor)() {
    }

    void Expression::VMIN(Asgn, dtor)() {
        am = Expression::AM_EQ;
    }

    void Expression::VMIN(PolyOp, dtor)() {
        ops.dtor();
    }

    void Expression::VMIN(Neg, dtor)() {
    }

    void Expression::VMIN(Cast, dtor)() {
        cast.dtor();
    }

    void Expression::VMIN(Num, dtor)() {
        num = nullptr;
    }

    void Expression::VMIN(VarRef, dtor)() {
        name = nullptr;
    }

    void Expression::VMIN(FuncCall, dtor)() {
        name = nullptr;
    }

    #define DEF_TYPE(NAME) \
        VTYPE_DEF(NAME, Expression) = { \
            Expression::VMIN(NAME, dtor), \
            Expression::VMIN(NAME, compile), \
        }
    #include "exprtypes.dsl.h"
    #undef DEF_TYPE


    bool Code::ctor() {
        TRY_B(stmts.ctor());
        TRY_B(scope.ctor());

        return false;
    }

    void Code::dtor() {
        stmts.dtor();
        scope.dtor();
    }

    bool Code::makeStatement(Statement** stmt) {
        TRY_B(stmts.append({}));

        *stmt = &stmts[-1];

        return false;
    }

    void Code::simplifyLastEmpty() {
        assert(stmts.getSize() > 0);

        if (stmts[-1].isEmpty()) {
            stmts.pop();
        }
    }

    bool Code::compile(FILE *ofile, const Function *func, const Program *prog) {
        // TODO
    }


    bool Statement::ctor() {
        VSETTYPE(this, Empty);

        return false;
    }

    // TODO

    bool Statement::ctorCompound() {
        VSETTYPE(this, Compound);

        return false;
    }

    bool Statement::ctorReturn() {
        VSETTYPE(this, Return);

        return false;
    }

    bool Statement::ctorLoop() {
        VSETTYPE(this, Loop);

        return false;
    }

    bool Statement::ctorCond() {
        VSETTYPE(this, Cond);

        return false;
    }

    bool Statement::ctorVarDecl() {
        VSETTYPE(this, VarDecl);

        return false;
    }

    bool Statement::ctorExpr() {
        VSETTYPE(this, Expr);

        return false;
    }

    bool Statement::ctorEmpty() {
        VSETTYPE(this, Empty);

        return false;
    }

    void Statement::dtor() {
        VCALL(this, dtor);
    }

    bool Statement::makeCode(Code** out_code) {
        *out_code = &code;

        return false;
    }

    bool Statement::makeAltCode(Code** out_altCode) {
        *out_altCode = &altCode;

        return false;
    }

    bool Statement::makeExpr(Expression** out_expr) {
        *out_expr = &expr;

        return false;
    }

    bool Statement::makeVar(Var** out_var) {
        *out_var = &var;

        return false;
    }

    #define DEF_TYPE(NAME) \
        bool Statement::is##NAME() const { \
            return VISINST(this, NAME); \
        }
    #include "stmttypes.dsl.h"
    #undef DEF_TYPE

    void Statement::VMIN(Compound, dtor)() {
        code.dtor();
    }

    void Statement::VMIN(Return, dtor)() {
        expr.dtor();
    }

    void Statement::VMIN(Loop, dtor)() {
        expr.dtor();
        code.dtor();
    }

    void Statement::VMIN(Cond, dtor)() {
        expr.dtor();
        code.dtor();
        altCode.dtor();
    }

    void Statement::VMIN(VarDecl, dtor)() {
        var.dtor();
        expr.dtor();
    }

    void Statement::VMIN(Expr, dtor)() {
        expr.dtor();
    }

    void Statement::VMIN(Empty, dtor)() {
    }


    bool Statement::VMIN(Compound, compile)(FILE *ofile) {
        TRY_B(code.compile(ofile));

        return false;
    }

    bool Statement::VMIN(Return, compile)(FILE *ofile) {
        TRY_B(expr.compile(ofile));

        fprintf(ofile, "ret ");

        return false;
    }

    bool Statement::VMIN(Loop, compile)(FILE *ofile) {
    }

    bool Statement::VMIN(Cond, compile)(FILE *ofile) {
    }

    bool Statement::VMIN(VarDecl, compile)(FILE *ofile) {
    }

    bool Statement::VMIN(Expr, compile)(FILE *ofile) {
    }

    bool Statement::VMIN(Empty, compile)(FILE *ofile) {
    }

    #define DEF_TYPE(NAME) \
        VTYPE_DEF(NAME, Statement) = { \
            Statement::VMIN(NAME, dtor), \
            Statement::VMIN(NAME, compile), \
        };
    #include "stmttypes.dsl.h"
    #undef DEF_TYPE


    bool Function::ctor() {
        TRY_B(args.ctor());
        TRY_B(code.ctor());
        rtype = {};
        name = nullptr;

        return false;
    }

    bool Function::ctor(TypeSpec new_rtype, const Token* new_name) {
        TRY_B(ctor());

        rtype = new_rtype;
        name = new_name;

        return false;
    }

    void Function::dtor() {
        args.dtor();
        code.dtor();
        rtype = {};
        name = nullptr;
    }

    bool Function::makeArg(Var** arg) {
        TRY_B(args.append({}));

        *arg = &args[-1];

        return false;
    }

    bool Function::makeCode(Code** out_code) {
        *out_code = &code;

        return false;
    }

    bool Function::registerArgs() {
        for (unsigned i = 0; i < args.getSize(); ++i) {
            TRY_B(code.scope.addVar(&args[i]));
        }

        return false;
    }

    bool Function::compile(FILE* ofile, const Program *prog) {
        fprintf(ofile, "\n$%.*s:\n", name->getLength(), name->getStr());

        TRY_B(code.compile(ofile, this, prog));

        fprintf(ofile, "ret  ; Force end of $%.*s\n\n", name->getLength(), name->getStr());
    }

    bool Function::isMain() const {
        const char MAIN_NAME[] = "main";

        return name->getLength() == sizeof(MAIN_NAME) &&
               rtype.getType() == TypeSpec::T_VOID &&
               strncmp(name->getStr(), MAIN_NAME, sizeof(MAIN_NAME)) == 0;
    }

    bool Program::ctor() {
        TRY_B(functions.ctor());

        return false;
    }

    void Program::dtor() {
        functions.dtor();
    }

    bool Program::makeFunction(Function** dest) {
        TRY_B(functions.append({}));

        *dest = &functions[-1];

        return false;
    }

    bool Program::compile(FILE* ofile) {
        bool seenMain = false;

        for (unsigned i = 0; i < functions.getSize(); ++i) {
            TRY_B(functions[i].compile(ofile, this));

            seenMain |= functions[i].isMain();
        }

        if (seenMain) {
            ERR("No main function present. (If it exists, make sure it doesn't return anything)");
        }

        return false;
    }

}

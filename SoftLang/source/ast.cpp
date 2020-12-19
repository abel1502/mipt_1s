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

    bool TypeSpec::ctor(Mask mask) {
        if (!isMaskUnambiguous(mask))
            return true;

        type = (Type_e)__builtin_ctz(mask);  // Almost txlib level magic)

        assert(type < TYPES_COUNT);

        return true;
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

    TypeSpec::Mask TypeSpec::getMask() const {
        return 1 << type;
    }

    bool TypeSpec::fitsMask(TypeSpec::Mask mask) const {
        return getMask() & mask;
    }

    constexpr bool TypeSpec::isMaskUnambiguous(TypeSpec::Mask mask) {
        return mask & (mask - 1);
    }

    //================================================================================

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

    bool Var::compile(FILE *ofile, const Scope *scope) {
        Scope::VarInfo vi = scope->getInfo(name);

        if (!vi.var) {
            //ERR("Unknown variable \"%.*s\"", name->getLength(), name->getStr());
            assert(false);

            return true;
        }

        TRY_B(ts.compile(ofile));
        fprintf(ofile, "[rzx+%u]", vi.offset);

        return false;
    }

    const TypeSpec Var::getType() const {
        return ts;
    }

    const Token *Var::getName() const {
        return name;
    }

    //================================================================================

    bool Scope::ctor() {
        TRY_B(vars.ctor());
        curOffset = 0;
        parent = nullptr;

        return false;
    }

    void Scope::dtor() {
        vars.dtor();
        parent = nullptr;
        curOffset = 0;
    }

    Scope::VarInfo Scope::getInfo(const Var *var) const {
        return getInfo(var->getName());
    }

    Scope::VarInfo Scope::getInfo(const Token *name) const {
        const Scope *cur = this;

        while (cur && !cur->vars.contains(name)) {
            cur = cur->parent;
        }

        if (!cur)
            return {(uint32_t)-1, nullptr};

        return cur->vars.get(name);
    }

    bool Scope::hasVar(const Var *var) const {
        return hasVar(var->getName());
    }

    bool Scope::hasVar(const Token *name) const {
        const Scope *cur = this;

        while (cur && !cur->vars.contains(name)) {
            cur = cur->parent;
        }

        return cur;
    }

    bool Scope::addVar(const Var *var) {
        if (vars.contains(var->getName())) {
            ERR("Redeclaration of variable \"%.*s\" within the same scope", var->getName()->getLength(), var->getName()->getStr());

            return true;
        }

        uint32_t offset = curOffset;

        curOffset += var->getType().getSize();

        TRY_B(vars.set(var->getName(), {offset, var}));

        return false;
    }

    void Scope::setParent(const Scope *new_parent) {
        parent = new_parent;
    }

    uint32_t Scope::getFrameSize() const {
        uint32_t result = 0;
        const Scope *cur = this;

        while (cur) {
            result += cur->curOffset;

            cur = cur->parent;
        }

        return result;
    }

    //================================================================================

    bool Expression::ctor() {
        children.ctor();
        typeMask = TypeSpec::AllMask;
        // TODO: zero-fill?
        return false;
    }

    bool Expression::ctorVoid() {
        TRY_B(ctor());

        VSETTYPE(this, Void);

        typeMask = TypeSpec::VoidMask;

        return false;
    }

    bool Expression::ctorAsgn() {
        TRY_B(ctor());

        VSETTYPE(this, Asgn);

        am = Expression::AM_EQ;
        TRY_B(children.ctor());

        return false;
    }

    bool Expression::ctorPolyOp() {
        TRY_B(ctor());

        VSETTYPE(this, PolyOp);

        TRY_B(ops.ctor());
        TRY_B(children.ctor());

        return false;
    }

    bool Expression::ctorNeg() {
        TRY_B(ctor());

        VSETTYPE(this, Neg);

        TRY_B(children.ctor());

        return false;
    }

    bool Expression::ctorCast(TypeSpec ts) {
        TRY_B(ctor());

        VSETTYPE(this, Cast);

        typeMask = ts.getMask();
        cast = ts;
        TRY_B(children.ctor());

        return false;
    }

    bool Expression::ctorNum(const Token *new_num) {
        TRY_B(ctor());

        VSETTYPE(this, Num);

        assert(new_num->isNum());
        num = new_num;

        if (num->isInteger()) {
            typeMask = TypeSpec::Int4Mask | TypeSpec::Int8Mask;
        } else {
            typeMask = TypeSpec::DblMask;
        }

        return false;
    }

    bool Expression::ctorVarRef(const Token *new_name) {
        TRY_B(ctor());

        VSETTYPE(this, VarRef);

        assert(new_name->isName());
        name = new_name;

        return false;
    }

    bool Expression::ctorFuncCall(const Token* new_name) {
        TRY_B(ctor());

        VSETTYPE(this, FuncCall);

        assert(new_name->isName());
        name = new_name;
        TRY_B(children.ctor());

        return false;
    }

    void Expression::dtor() {
        children.dtor();
        typeMask = TypeSpec::NoneMask;
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

        Expression singleChild{};
        memcpy(&singleChild, &children[0], sizeof(Expression));

        children.pop();  // Destructor intentionally not called

        dtor();

        memcpy(this, &singleChild, sizeof(Expression));

        // memset(&singleChild, 0, sizeof(Expression));  // Would have done to avoid implicit destruction, but
                                                         // thanks the C coding style we don't have it at all

        // And now the singleChild get silently removed, with no destructors called...
    }

    TypeSpec::Mask Expression::deduceType(TypeSpec::Mask mask, Scope *scope, const Program *prog) {
        if (!typeMask || TypeSpec::isMaskUnambiguous(typeMask)) {  // TODO: Maybe a different condition
            return typeMask;
        }

        typeMask &= mask;

        return typeMask &= VCALL(this, deduceType, scope, prog);
    }

    bool Expression::compile(FILE *ofile, Scope *scope, const Program *prog) {
        return VCALL(this, compile, ofile, scope, prog);
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


    TypeSpec::Mask Expression::VMIN(Void, deduceType)(Scope *, const Program *) {
        return TypeSpec::VoidMask;
    }

    TypeSpec::Mask Expression::VMIN(Asgn, deduceType)(Scope *scope, const Program *prog) {
        assert(children.getSize() == 2);

        typeMask &= children[0].deduceType(typeMask, scope, prog);
        typeMask &= children[1].deduceType(typeMask, scope, prog);

        return typeMask;
    }

    TypeSpec::Mask Expression::VMIN(PolyOp, deduceType)(Scope *scope, const Program *prog) {
        for (unsigned i = 0; typeMask && i < children.getSize(); ++i) {
            typeMask &= children[i].deduceType(typeMask, scope, prog);
        }

        return typeMask;
    }

    TypeSpec::Mask Expression::VMIN(Neg, deduceType)(Scope *scope, const Program *prog) {
        assert(children.getSize() == 1);

        return children[0].deduceType(typeMask, scope, prog);
    }

    TypeSpec::Mask Expression::VMIN(Cast, deduceType)(Scope *, const Program *) {
        assert(children.getSize() == 1);

        /*
        // Actually, this should probably be done during compilation, not type deduction
        if (!children[0].deduceType(TypeSpec::AllMask & ~TypeSpec::VoidMask, scope, prog)) {
            return TypeSpec::NoneMask;
        }
        */

        return cast.getMask();
    }

    TypeSpec::Mask Expression::VMIN(Num, deduceType)(Scope *, const Program *) {
        assert(num->isNum());

        if (num->isInteger()) {
            return TypeSpec::Int4Mask | TypeSpec::Int8Mask;
        } else {
            return TypeSpec::DblMask;
        }
    }

    TypeSpec::Mask Expression::VMIN(VarRef, deduceType)(Scope *scope, const Program *) {
        assert(name->isName());

        Scope::VarInfo vi = scope->getInfo(name);

        if (!vi.var) {
            //ERR("Unknown variable, type indeterminable: \"%.*s\"", name->getLength(), name->getStr());

            return TypeSpec::NoneMask;
        }

        return vi.var->getType().getMask();
    }

    TypeSpec::Mask Expression::VMIN(FuncCall, deduceType)(Scope *, const Program *prog) {
        assert(name->isName());

        // TODO: Pseudofuncs!!!!

        const Function *func = prog->getFunction(name);

        if (!func) {
            //ERR("Unknown function, return type indeterminable: \"%.*s\"", name->getLength(), name->getStr());

            return TypeSpec::NoneMask;
        }

        return func->getRtype().getMask();  // TODO: ?
    }


    bool Expression::VMIN(Void, compile)(FILE *ofile, Scope *scope, const Program *prog) {
    }

    bool Expression::VMIN(Asgn, compile)(FILE *ofile, Scope *scope, const Program *prog) {
    }

    bool Expression::VMIN(PolyOp, compile)(FILE *ofile, Scope *scope, const Program *prog) {
    }

    bool Expression::VMIN(Neg, compile)(FILE *ofile, Scope *scope, const Program *prog) {
    }

    bool Expression::VMIN(Cast, compile)(FILE *ofile, Scope *scope, const Program *prog) {
    }

    bool Expression::VMIN(Num, compile)(FILE *ofile, Scope *scope, const Program *prog) {
    }

    bool Expression::VMIN(VarRef, compile)(FILE *ofile, Scope *scope, const Program *prog) {
    }

    bool Expression::VMIN(FuncCall, compile)(FILE *ofile, Scope *scope, const Program *prog) {
    }


    #define DEF_TYPE(NAME) \
        VTYPE_DEF(NAME, Expression) = { \
            Expression::VMIN(NAME, dtor), \
            Expression::VMIN(NAME, deduceType), \
            Expression::VMIN(NAME, compile), \
        }
    #include "exprtypes.dsl.h"
    #undef DEF_TYPE

    //================================================================================

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
            stmts[-1].dtor();
            stmts.pop();
        }
    }

    bool Code::compile(FILE *ofile, TypeSpec rtype, const Program *prog) {
        for (unsigned i = 0; i < stmts.getSize(); ++i) {
            TRY_B(stmts[i].compile(ofile, &scope, rtype, prog));
        }

        return false;
    }

    Scope *Code::getScope() {
        return &scope;
    }

    //================================================================================

    bool Statement::ctor() {
        VSETTYPE(this, Empty);

        return false;
    }

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

    bool Statement::compile(FILE *ofile, Scope *scope, TypeSpec rtype, const Program *prog) {
        return VCALL(this, compile, ofile, scope, rtype, prog);
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


    bool Statement::VMIN(Compound, compile)(FILE *ofile, Scope *scope, TypeSpec rtype, const Program *prog) {
        TRY_B(code.compile(ofile, rtype, prog));

        return false;
    }

    bool Statement::VMIN(Return, compile)(FILE *ofile, Scope *scope, TypeSpec rtype, const Program *prog) {
        if (rtype.type == TypeSpec::T_VOID) {
            if (!expr.isVoid()) {
                ERR("Void functions mustn't return values");

                return true;
            }  // TODO: Maybe also compile void expression, but make it trivial?
        } else {
            TRY_B(expr.compile(ofile, scope, prog));  // TODO: Allowed type mask and other options
        }

        fprintf(ofile, "ret\n");

        return false;
    }

    bool Statement::VMIN(Loop, compile)(FILE *ofile, Scope *scope, TypeSpec rtype, const Program *prog) {
        fprintf(ofile,
                "; while (\n"
                "$__loop_in_%p:\n"  // TODO: Maybe change to something more adequate
                , this);

        TRY_B(expr.compile(ofile, scope, prog));  // TODO: Expr stuff

        fprintf(ofile,
                "jt dwl:$__loop_out_%p\n"
                "; ) {\n"
                , this);

        TRY_B(code.compile(ofile, rtype, prog));

        fprintf(ofile,
                "jmp dwl:$__loop_in_%p\n"
                "$__loop_out_%p:\n"
                "; }\n"
                , this, this);

        return false;
    }

    bool Statement::VMIN(Cond, compile)(FILE *ofile, Scope *scope, TypeSpec rtype, const Program *prog) {
        // TODO: Same as for the loop
        // TODO: Maybe simplify the empty else

        fprintf(ofile,
                "; if (\n");

        TRY_B(expr.compile(ofile, scope, prog));  // TODO: Expr stuff

        fprintf(ofile,
                "jt dwl:$__cond_t_%p\n"
                "jmp dwl:$__cond_f_%p\n"
                "; ) {\n"
                , this, this);

        TRY_B(code.compile(ofile, rtype, prog));

        fprintf(ofile,
                "jmp dwl:$__cond_end_%p\n"
                "; } else {\n"
                "$__cond_f_%p:\n"
                , this, this);

        TRY_B(altCode.compile(ofile, rtype, prog));

        fprintf(ofile,
                "$__cond_end_%p:\n"
                "; }\n"
                , this);

        return false;
    }

    bool Statement::VMIN(VarDecl, compile)(FILE *ofile, Scope *scope, TypeSpec rtype, const Program *prog) {
        TRY_B(scope->addVar(&var));

        // TODO: More precise logging (type, etc.)
        fprintf(ofile,
                "; var %.*s\n",
                var.getName()->getLength(), var.getName()->getStr());

        if (expr.isVoid())
            return false;

        TRY_B(expr.compile(ofile, scope, prog));  // TODO: Expr again

        fprintf(ofile,
                "; = \n"
                "pop ");
        TRY_B(var.compile(ofile, scope));
        fprintf(ofile, "\n");

        return false;
    }

    bool Statement::VMIN(Expr, compile)(FILE *ofile, Scope *scope, TypeSpec rtype, const Program *prog) {
        TRY_B(expr.compile(ofile, scope, prog));  // TODO: Expr bla-bla-bla

        fprintf(ofile, "popv\n");
    }

    bool Statement::VMIN(Empty, compile)(FILE *ofile, Scope *scope, TypeSpec rtype, const Program *prog) {
    }

    #define DEF_TYPE(NAME) \
        VTYPE_DEF(NAME, Statement) = { \
            Statement::VMIN(NAME, dtor), \
            Statement::VMIN(NAME, compile), \
        };
    #include "stmttypes.dsl.h"
    #undef DEF_TYPE

    //================================================================================

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
            TRY_B(code.getScope()->addVar(&args[i]));
        }

        return false;
    }

    bool Function::compile(FILE* ofile, const Program *prog) {
        fprintf(ofile, "\n$__func_%.*s:\n", name->getLength(), name->getStr());

        TRY_B(code.compile(ofile, rtype, prog));

        fprintf(ofile, "ret  ; Force end of $%.*s\n\n", name->getLength(), name->getStr());
    }

    bool Function::isMain() const {
        const char MAIN_NAME[] = "main";

        return name->getLength() == sizeof(MAIN_NAME) &&
               rtype.type == TypeSpec::T_VOID &&
               strncmp(name->getStr(), MAIN_NAME, sizeof(MAIN_NAME)) == 0;
    }

    TypeSpec Function::getRtype() const {
        return rtype;
    }

    const Token *Function::getName() const {
        return name;
    }

    //================================================================================

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

    const Function *Program::getFunction(const Token *name) const {
        for (unsigned i = 0; i < functions.getSize(); ++i) {
            const Token *curName = functions[i].getName();

            // TODO: Encapsulate name token comparison into the token itself
            if (name->getLength() == curName->getLength() &&
                strncmp(name->getStr(), curName->getStr(), name->getLength()) == 0) {

                return &functions[i];
            }
        }

        return nullptr;
    }

}

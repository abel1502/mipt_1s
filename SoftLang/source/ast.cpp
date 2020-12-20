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
        //printf("DEBUG: 0x%x, %s unambiguous\n", mask, isMaskUnambiguous(mask) ? "is" : "isn't");

        if (!isMaskUnambiguous(mask))
            return true;

        type = (Type_e)__builtin_ctz(mask);  // Almost txlib level magic)

        assert(type < TYPES_COUNT);

        return false;
    }

    void TypeSpec::dtor() {
        type = TypeSpec::T_VOID;
    }

    bool TypeSpec::compile(FILE *ofile) const {
        switch (type) {
        case TypeSpec::T_DBL:
            fprintf(ofile, "df:");
            break;

        case TypeSpec::T_INT4:
            fprintf(ofile, "dwl:");
            break;

        case TypeSpec::T_INT8:
            fprintf(ofile, "qw:");
            break;

        case TypeSpec::T_VOID:
            ERR("Void is not a valid type");
            // FALLTHROUGH
        default:
            return true;
        }

        return false;
    }

    void TypeSpec::reconstruct(FILE *ofile) const {
        switch (type) {
        case TypeSpec::T_DBL:
            fprintf(ofile, "dbl:");
            break;

        case TypeSpec::T_INT4:
            fprintf(ofile, "int4:");
            break;

        case TypeSpec::T_INT8:
            fprintf(ofile, "int8:");
            break;

        case TypeSpec::T_VOID:
            fprintf(ofile, "???:");
            ERR("Void is not a valid type");
            // FALLTHROUGH
        default:
            assert(false);
        }
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
        return !(mask & (mask - 1)) && mask;
    }

    //================================================================================

    // Reserved for VarInfo

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
        name = nullptr;
    }

    bool Var::compile(FILE *ofile, const Scope *scope) const {
        return compile(ofile, scope->getInfo(name));
    }

    bool Var::compile(FILE *ofile, VarInfo vi) {
        if (!vi.var) {
            //ERR("Unknown variable \"%.*s\"", name->getLength(), name->getStr());
            assert(false);

            return true;
        }

        TRY_B(vi.var->ts.compile(ofile));
        fprintf(ofile, "[rz+%u]", vi.offset);

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

    VarInfo Scope::getInfo(const Var *var) const {
        return getInfo(var->getName());
    }

    VarInfo Scope::getInfo(const Token *name) const {
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
        //printf("New parent: %p -> %p\n", parent, new_parent);

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

        if (vtable_)
            VCALL(this, dtor);
    }

    bool Expression::makeChild(Expression** child) {
        TRY_B(children.append({}));

        *child = &children[-1];

        return false;
    }

    void Expression::popChild() {
        children.pop();
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
        typeMask &= VCALL(this, deduceType, scope, prog);

        return typeMask;
    }

    bool Expression::compile(FILE *ofile, Scope *scope, const Program *prog) {
        /*printf("; EXPR COMPILE\n");
        #define DEF_TYPE(NAME)                          \
            if (vtable_ == &Expression::VTYPE(NAME)) {  \
                printf("; ->" #NAME "\n");                          \
            } else
        #include "exprtypes.dsl.h"
        #undef DEF_TYPE
        {
            ERR("; ->UNKNOWN TYPE\n");
            assert(false);
        }*/
        return VCALL(this, compile, ofile, scope, prog);
    }

    bool Expression::compileVarRecepient(FILE *ofile, Scope *scope, const Program *) {
        assert(isVarRef());

        fprintf(ofile,
                "dup\n"  // TODO: Maybe not dup here
                "pop ");

        VarInfo vi = scope->getInfo(name);
        TRY_B(Var::compile(ofile, vi));

        fprintf(ofile, "\n");

        return false;
    }

    void Expression::reconstruct(FILE *ofile) const {
        fprintf(ofile, "(");
        VCALL(this, reconstruct, ofile);
        fprintf(ofile, ")");
    }

    TypeSpec::Mask Expression::getPseudofuncRtypeMask() const {
        // NAME doesn't include the leading '_', which causes slight changes in string-name comparison;
        #define DEF_PFUNC(RTYPECAP, RTYPEWORD, NAME, COMPILECODE)               \
            if (sizeof(#NAME) == name->getLength() &&                           \
                strncmp(name->getStr() + 1, #NAME, sizeof(#NAME) - 1) == 0) {   \
                                                                                \
                return TypeSpec::RTYPEWORD##Mask;                               \
            }
        #include "pseudofuncs.dsl.h"
        #undef DEF_PFUNC

        ERR("Unknown pseudo-function, return type indeterminable: \"%.*s\"", name->getLength(), name->getStr());

        return TypeSpec::NoneMask;
    }

    bool Expression::compilePseudofunc(FILE *ofile, Scope *scope, const Program *prog) {
        #define PF_DEDUCE_CHILD_TYPE(IND, MASK) \
            children[IND].deduceType(MASK, scope, prog)

        #define PF_REQUIRE_CHILD_TYPE(IND, TYPEWORD)                    \
            if (!PF_DEDUCE_CHILD_TYPE(IND, TypeSpec::TYPEWORD##Mask)) { \
                ERR("Argument #%u for %.*s must be %s", IND,            \
                    name->getLength(), name->getStr(), #TYPEWORD);      \
                                                                        \
                return true;                                            \
            }

        #define PF_VERIFY_ARGCNT(CNT)                                   \
            if (children.getSize() != CNT) {                            \
                ERR("Pseudofunction %.*s takes exactly %u arguments",   \
                    name->getLength(), name->getStr(), CNT);            \
                                                                        \
                return true;                                            \
            }

        #define PF_COMPILE_CHILD(IND) \
            TRY_B(children[IND].compile(ofile, scope, prog));

        #define PF_O(CODE, ...) \
            fprintf(ofile, CODE, ##__VA_ARGS__);

        #define DEF_PFUNC(RTYPECAP, RTYPEWORD, NAME, COMPILECODE)               \
            if (sizeof(#NAME) == name->getLength() &&                           \
                strncmp(name->getStr() + 1, #NAME, sizeof(#NAME) - 1) == 0) {   \
                                                                                \
                COMPILECODE;                                                    \
                return false;                                                   \
            }
        #include "pseudofuncs.dsl.h"
        #undef DEF_PFUNC

        ERR("Unknown pseudo-function: \"%.*s\"", name->getLength(), name->getStr());

        return true;

        #undef PF_DEDUCE_CHILD_TYPE
        #undef PF_VERIFY_ARGCNT
        #undef PF_REQUIRE_CHILD_TYPE
        #undef PF_COMPILE_CHILD
        #undef PF_O
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

        typeMask &= children[0].deduceType(typeMask, scope, prog);  // Result is always either unambiguous or none
        typeMask &= children[1].deduceType(typeMask, scope, prog);

        // So, whenever typeMask ends up unambiguous, the children already know about it

        return typeMask;
    }

    TypeSpec::Mask Expression::VMIN(PolyOp, deduceType)(Scope *scope, const Program *prog) {
        bool isCmp = false;

        if (ops.getSize() > 0) {
            #pragma GCC diagnostic push
            #pragma GCC diagnostic ignored "-Wswitch-enum"

            switch (ops[0]) {
            case Expression::OP_EQ:
            case Expression::OP_NEQ:
            case Expression::OP_GEQ:
            case Expression::OP_LEQ:
            case Expression::OP_GT:
            case Expression::OP_LT:
                isCmp = true;  // A comparison always return Int4, no matter what is consists of, but its children should still be of a single and determinable type
                typeMask = TypeSpec::AllMask & ~TypeSpec::VoidMask;

                if (children.getSize() > 2) {
                    ERR("Comparisons of more than two values aren't yet implemented, sorry");

                    return TypeSpec::NoneMask;
                }

                break;

            default:
                break;
            }

            #pragma GCC diagnostic pop
        }

        for (unsigned i = 0; typeMask && i < children.getSize(); ++i) {
            typeMask &= children[i].deduceType(typeMask, scope, prog);
        }

        if (TypeSpec::isMaskUnambiguous(typeMask)) {
            for (unsigned i = 0; i < children.getSize(); ++i) {
                children[i].deduceType(typeMask, scope, prog);  // Pushing through for them to know who to compile as
            }
        }

        if (isCmp) {
            if (TypeSpec::isMaskUnambiguous(typeMask)) {
                typeMask = TypeSpec::Int4Mask;
            } else {
                typeMask = TypeSpec::NoneMask;
            }
        }

        return typeMask;
    }

    TypeSpec::Mask Expression::VMIN(Neg, deduceType)(Scope *scope, const Program *prog) {
        assert(children.getSize() == 1);

        return children[0].deduceType(typeMask, scope, prog);  // Again, no manual pushing required
    }

    TypeSpec::Mask Expression::VMIN(Cast, deduceType)(Scope *scope, const Program *prog) {
        assert(children.getSize() == 1);

        /*
        // Actually, this should probably be done during compilation, not type deduction
        if (!children[0].deduceType(TypeSpec::AllMask & ~TypeSpec::VoidMask, scope, prog)) {
            return TypeSpec::NoneMask;
        }
        */

        // The only requirement posed on the cast subject is to be non-void (at least at this point)
        return cast.getMask() & children[0].deduceType(TypeSpec::AllMask & ~TypeSpec::VoidMask, scope, prog);
    }

    TypeSpec::Mask Expression::VMIN(Num, deduceType)(Scope *, const Program *) {
        assert(num->isNum());

        if (num->isInteger()) {
            return TypeSpec::Int4Mask | TypeSpec::Int8Mask /*| TypeSpec::DblMask*/;  // TODO: Not sure if we should consider double possible here
        } else {
            return TypeSpec::DblMask;
        }
    }

    TypeSpec::Mask Expression::VMIN(VarRef, deduceType)(Scope *scope, const Program *) {
        assert(name->isName());

        VarInfo vi = scope->getInfo(name);

        if (!vi.var) {
            ERR("Unknown variable, type indeterminable: \"%.*s\"", name->getLength(), name->getStr());

            return TypeSpec::NoneMask;
        }

        return vi.var->getType().getMask();
    }

    TypeSpec::Mask Expression::VMIN(FuncCall, deduceType)(Scope *, const Program *prog) {
        assert(name->isName());

        if (name->getLength() >= 1 && name->getStr()[0] == '_') {
            return getPseudofuncRtypeMask();
        }

        const Function *func = prog->getFunction(name);

        if (!func) {
            ERR("Unknown function, return type indeterminable: \"%.*s\"", name->getLength(), name->getStr());

            return TypeSpec::NoneMask;
        }

        // Argument masks and count checks should be applied later

        return func->getRtype().getMask();  // TODO: ?
    }


    bool Expression::VMIN(Void, compile)(FILE *, Scope *, const Program *) {
        assert(children.getSize() == 0);

        // Let's leave it trivial for now, I guess
        TypeSpec exprType{};
        TRY_BC(exprType.ctor(typeMask), ERR("Ambiguous type"));
        TRY_B(exprType.type != TypeSpec::T_VOID);
        exprType.dtor();

        return false;
    }

    bool Expression::VMIN(Asgn, compile)(FILE *ofile, Scope *scope, const Program *prog) {
        assert(children.getSize() == 2);

        TypeSpec exprType{};
        TRY_BC(exprType.ctor(typeMask), ERR("Ambiguous type"));
        TRY_B(exprType.type == TypeSpec::T_VOID);

        if (am != Expression::AM_EQ) {
            TRY_B(children[0].compile(ofile, scope, prog));
        }

        TRY_B(children[1].compile(ofile, scope, prog));

        if (am != Expression::AM_EQ) {
            switch (am) {
            case AM_ADDEQ:
                fprintf(ofile, "add ");
                break;

            case AM_SUBEQ:
                fprintf(ofile, "sub ");
                break;

            case AM_MULEQ:
                fprintf(ofile, "mul ");
                break;

            case AM_DIVEQ:
                fprintf(ofile, "div ");
                break;

            case AM_MODEQ:
                fprintf(ofile, "mod ");

                TRY_BC(exprType.type == TypeSpec::T_DBL, ERR("Remainder can't be computed for non-integral types"));
                break;

            case OP_EQ:
            default:
                assert(false);
                return true;
            }

            TRY_B(exprType.compile(ofile));
            fprintf(ofile, "\n");
        }

        TRY_B(children[0].compileVarRecepient(ofile, scope, prog));

        exprType.dtor();

        return false;
    }

    bool Expression::VMIN(PolyOp, compile)(FILE *ofile, Scope *scope, const Program *prog) {
        assert(children.getSize() > 0);
        //assert(children.getSize() > 1);

        TypeSpec exprType{};
        TRY_BC(exprType.ctor(typeMask), ERR("Ambiguous type"));
        TRY_B(exprType.type == TypeSpec::T_VOID);

        TRY_B(children[0].compile(ofile, scope, prog));

        for (unsigned i = 0; i + 1 < children.getSize(); i++) {
            // We do rely onall the operators being left-to-right evaluated, so
            // take caution when adding new ones that don't follow this rule
            // (such as **, for example)

            TRY_B(children[i + 1].compile(ofile, scope, prog));

            switch (ops[i]) {
            case OP_EQ:
                fprintf(ofile, "ce ");
                break;

            case OP_NEQ:
                fprintf(ofile, "cne ");
                break;

            case OP_GEQ:
                fprintf(ofile, "cge ");
                break;

            case OP_LEQ:
                fprintf(ofile, "cle ");
                break;

            case OP_LT:
                fprintf(ofile, "cl ");
                break;

            case OP_GT:
                fprintf(ofile, "cg ");
                break;

            case OP_ADD:
                fprintf(ofile, "add ");
                break;

            case OP_SUB:
                fprintf(ofile, "sub ");
                break;

            case OP_MUL:
                fprintf(ofile, "mul ");
                break;

            case OP_DIV:
                fprintf(ofile, "div ");
                break;

            case OP_MOD:
                fprintf(ofile, "mod ");
                break;

            default:
                assert(false);
                return true;
            }

            // For cmp's exprType is Int4, so we need the child's one
            if (typeMask != children[i].typeMask) {
                TRY_B(exprType.ctor(children[i].typeMask));
            }

            exprType.compile(ofile);
            fprintf(ofile, "\n");

            // TODO: Eventually should change poly-comparison handling to the pythonic way.
            // TODO: For now, in fact, I should probably forbid a == b == c stuff at all
        }

        exprType.dtor();

        return false;
    }

    bool Expression::VMIN(Neg, compile)(FILE *ofile, Scope *scope, const Program *prog) {
        assert(children.getSize() == 1);

        TypeSpec exprType{};
        TRY_BC(exprType.ctor(typeMask), ERR("Ambiguous type"));
        TRY_B(exprType.type == TypeSpec::T_VOID);

        TRY_B(children[0].compile(ofile, scope, prog));
        fprintf(ofile, "neg ");
        TRY_B(exprType.compile(ofile));
        fprintf(ofile, "\n");

        exprType.dtor();

        return false;
    }

    bool Expression::VMIN(Cast, compile)(FILE *ofile, Scope *scope, const Program *prog) {
        assert(children.getSize() == 1);

        TypeSpec exprType{};
        TRY_BC(exprType.ctor(typeMask), ERR("Ambiguous type"));

        TypeSpec childType{};
        TypeSpec::Mask tmpMask = children[0].deduceType(TypeSpec::AllMask, scope, prog);

        if (tmpMask == (TypeSpec::Int4Mask | TypeSpec::Int8Mask)) {
            // A workaround for dbl:123 and such
            tmpMask = children[0].deduceType(TypeSpec::Int4Mask, scope, prog);
        }

        if (childType.ctor(tmpMask)){

            ERR("Ambiguous type");
        }

        TRY_B(children[0].compile(ofile, scope, prog));

        switch (exprType.type) {
        case TypeSpec::T_VOID:
            ERR("Can't cast to void");
            return true;

        case TypeSpec::T_DBL:
            switch (childType.type) {
            case TypeSpec::T_VOID:
                fprintf(ofile, "push ");
                TRY_B(exprType.compile(ofile));
                fprintf(ofile, "0\n");
                break;

            case TypeSpec::T_DBL:
                break;

            case TypeSpec::T_INT4:
                fprintf(ofile, "d2i\n");
                break;

            case TypeSpec::T_INT8:
                fprintf(ofile, "d2l\n");  // TODO: Implement
                break;

            default:
                assert(false);
                return true;
            }
            break;

        case TypeSpec::T_INT8:
            switch (childType.type) {
            case TypeSpec::T_VOID:
                fprintf(ofile, "push ");
                TRY_B(exprType.compile(ofile));
                fprintf(ofile, "0\n");
                break;

            case TypeSpec::T_DBL:
                fprintf(ofile, "l2d\n");  // TODO: Implement
                break;

            case TypeSpec::T_INT4:
                break;

            case TypeSpec::T_INT8:
                // Kind of works automatically
                break;

            default:
                assert(false);
                return true;
            }
            break;

        case TypeSpec::T_INT4:
            switch (childType.type) {
            case TypeSpec::T_VOID:
                fprintf(ofile, "push ");
                TRY_B(exprType.compile(ofile));
                fprintf(ofile, "0\n");
                break;

            case TypeSpec::T_DBL:
                fprintf(ofile, "i2d\n");
                break;

            case TypeSpec::T_INT4:
                // Kind of works automatically, although a bit trickier
                break;

            case TypeSpec::T_INT8:
                break;

            default:
                assert(false);
                return true;
            }
            break;

        default:
            assert(false);
            return true;
        }

        childType.dtor();
        exprType.dtor();

        return false;
    }

    bool Expression::VMIN(Num, compile)(FILE *ofile, Scope *, const Program *) {
        assert(children.getSize() == 0);

        TypeSpec exprType{};
        TRY_BC(exprType.ctor(typeMask), ERR("Ambiguous type"));
        //TRY_B(exprType.type == TypeSpec::T_VOID);

        switch (exprType.type) {
        case TypeSpec::T_VOID:
            ERR("Void isn't a number");
            return true;

        case TypeSpec::T_INT4:
        case TypeSpec::T_INT8:
            fprintf(ofile, "push ");
            exprType.compile(ofile);
            fprintf(ofile, "%llu\n", num->asInt());

            break;

        case TypeSpec::T_DBL:
            fprintf(ofile, "push ");
            exprType.compile(ofile);
            fprintf(ofile, "%lg\n", num->asDbl());

            break;

        default:
            assert(false);
            return true;
        }

        exprType.dtor();

        return false;
    }

    bool Expression::VMIN(VarRef, compile)(FILE *ofile, Scope *scope, const Program *) {
        assert(children.getSize() == 0);

        TypeSpec exprType{};
        TRY_BC(exprType.ctor(typeMask), ERR("Ambiguous type"));
        TRY_B(exprType.type == TypeSpec::T_VOID);

        VarInfo vi = scope->getInfo(name);

        assert(vi.var);  // If it didn't exist, type deduction would have already failed
        /*if (!vi.var) {
            ERR("Unknown variable: \"%.*s\"", name->getLength(), name->getStr());
            return true;
        }*/

        fprintf(ofile, "push ");
        TRY_B(Var::compile(ofile, vi));
        fprintf(ofile, "\n");

        exprType.dtor();

        return false;
    }

    bool Expression::VMIN(FuncCall, compile)(FILE *ofile, Scope *scope, const Program *prog) {
        TypeSpec exprType{};
        TRY_BC(exprType.ctor(typeMask), ERR("Ambiguous type"));

        if (name->getLength() >= 1 && name->getStr()[0] == '_') {
            return compilePseudofunc(ofile, scope, prog);
        }

        const Function *func = prog->getFunction(name);

        TRY_BC(!func, ERR("Unknown function: \"%.*s\"", name->getLength(), name->getStr()));

        assert(func->getRtype().type == exprType.type);  // Should have been guaranteed by the deduceType

        const Vector<Var> *args = func->getArgs();

        for (unsigned i = 0; i < args->getSize(); ++i) {
            TypeSpec::Mask childMask = children[i].deduceType((*args)[i].getType().getMask(), scope, prog);

            if (!childMask) {
                ERR("Bad argument type");
                return true;
            }

            if (!TypeSpec::isMaskUnambiguous(childMask)) {  // TODO: Review usages of this function and maybe remove unnecessary zero checks
                ERR("Ambiguous argument type");
                return true;
            }

            assert(childMask != TypeSpec::VoidMask);

            children[i].compile(ofile, scope, prog);
        }

        fprintf(ofile,
                "push dwl:rz\n"
                "push dwl:%u\n"
                "add dwl:\n"
                "pop dwl:rz\n"
                "call dwl:$__func_%.*s\n"
                "push dwl:rz\n"
                "push dwl:%u\n"
                "sub dwl:\n"
                "pop dwl:rz\n"
                , scope->getFrameSize(), name->getLength(), name->getStr(), scope->getFrameSize());

        exprType.dtor();

        return false;
    }


    void Expression::VMIN(Void, reconstruct)(FILE *ofile) const {
        fprintf(ofile, "???");
        ERR("Void expressions don't exist");
        assert(false);
    }

    void Expression::VMIN(Asgn, reconstruct)(FILE *ofile) const {
        children[0].reconstruct(ofile);

        switch (am) {
        case Expression::AM_EQ:
            fprintf(ofile, " = ");
            break;

        case Expression::AM_ADDEQ:
            fprintf(ofile, " += ");
            break;

        case Expression::AM_SUBEQ:
            fprintf(ofile, " -= ");
            break;

        case Expression::AM_MULEQ:
            fprintf(ofile, " -= ");
            break;

        case Expression::AM_DIVEQ:
            fprintf(ofile, " /= ");
            break;

        case Expression::AM_MODEQ:
            fprintf(ofile, " %%= ");
            break;

        default:
            assert(false);
            break;
        }

        children[1].reconstruct(ofile);
    }

    void Expression::VMIN(PolyOp, reconstruct)(FILE *ofile) const {
        assert(children.getSize() > 0);

        children[0].reconstruct(ofile);

        for (unsigned i = 1; i < children.getSize(); ++i) {
            switch (ops[i - 1]) {
            case OP_EQ:
                fprintf(ofile, " == ");
                break;

            case OP_NEQ:
                fprintf(ofile, " != ");
                break;

            case OP_GEQ:
                fprintf(ofile, " >= ");
                break;

            case OP_LEQ:
                fprintf(ofile, " <= ");
                break;

            case OP_LT:
                fprintf(ofile, " < ");
                break;

            case OP_GT:
                fprintf(ofile, " > ");
                break;

            case OP_ADD:
                fprintf(ofile, " + ");
                break;

            case OP_SUB:
                fprintf(ofile, " - ");
                break;

            case OP_MUL:
                fprintf(ofile, " * ");
                break;

            case OP_DIV:
                fprintf(ofile, "/ ");
                break;

            case OP_MOD:
                fprintf(ofile, " %% ");
                break;

            default:
                assert(false);
                return;
            }

            children[i].reconstruct(ofile);
        }
    }

    void Expression::VMIN(Neg, reconstruct)(FILE *ofile) const {
        fprintf(ofile, "-");

        children[0].reconstruct(ofile);
    }

    void Expression::VMIN(Cast, reconstruct)(FILE *ofile) const {
        cast.reconstruct(ofile);

        children[0].reconstruct(ofile);
    }

    void Expression::VMIN(Num, reconstruct)(FILE *ofile) const {
        if (num->isInteger()) {
            fprintf(ofile, "%llu", num->asInt());
        } else {
            double tmp = num->asDbl();

            fprintf(ofile, "%lg", tmp);

            if (tmp == (double)(unsigned long long)tmp) {
                fprintf(ofile, ".");
            }
        }
    }

    void Expression::VMIN(VarRef, reconstruct)(FILE *ofile) const {
        fprintf(ofile, "%.*s", name->getLength(), name->getStr());
    }

    void Expression::VMIN(FuncCall, reconstruct)(FILE *ofile) const {
        fprintf(ofile, "%.*s(", name->getLength(), name->getStr());

        for (unsigned i = 0; i < children.getSize(); ++i) {
            children[i].reconstruct(ofile);
        }

        fprintf(ofile, ")");
    }


    #define DEF_TYPE(NAME) \
        VTYPE_DEF(NAME, Expression) = { \
            Expression::VMIN(NAME, dtor), \
            Expression::VMIN(NAME, deduceType), \
            Expression::VMIN(NAME, compile), \
            Expression::VMIN(NAME, reconstruct), \
        };
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

    void Code::popStatement() {
        stmts.pop();
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

    void Code::reconstruct(FILE *ofile, unsigned indent) const {
        for (unsigned i = 0; i < stmts.getSize(); ++i) {
            stmts[i].reconstruct(ofile, indent);
        }
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
        if (vtable_)
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

    void Statement::reconstruct(FILE *ofile, unsigned indent) const {
        for (unsigned i = 0; i < indent; ++i)
            fprintf(ofile, "    ");

        VCALL(this, reconstruct, ofile, indent);

        fprintf(ofile, "\n");
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
        code.getScope()->setParent(scope);
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
            expr.deduceType(rtype.getMask(), scope, prog);

            TRY_B(expr.compile(ofile, scope, prog));
        }

        fprintf(ofile, "ret\n");

        return false;
    }

    bool Statement::VMIN(Loop, compile)(FILE *ofile, Scope *scope, TypeSpec rtype, const Program *prog) {
        fprintf(ofile,
                "; while (\n"
                "$__loop_in_%p:\n"  // TODO: Maybe change to something more adequate
                , this);

        TRY_BC(expr.deduceType(TypeSpec::AllMask & ~TypeSpec::VoidMask, scope, prog) != TypeSpec::Int4Mask,
               ERR("Ambiguous type"));

        TRY_B(expr.compile(ofile, scope, prog));

        fprintf(ofile,
                "jf dwl:$__loop_out_%p\n"
                "; ) {\n"
                , this);

        code.getScope()->setParent(scope);
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

        TRY_BC(expr.deduceType(TypeSpec::AllMask & ~TypeSpec::VoidMask, scope, prog) != TypeSpec::Int4Mask,
               ERR("Ambiguous type"));

        TRY_B(expr.compile(ofile, scope, prog));

        fprintf(ofile,
                "jt dwl:$__cond_t_%p\n"
                "jmp dwl:$__cond_f_%p\n"
                "; ) {\n"
                "$__cond_t_%p:\n"
                , this, this, this);

        code.getScope()->setParent(scope);
        TRY_B(code.compile(ofile, rtype, prog));

        fprintf(ofile,
                "jmp dwl:$__cond_end_%p\n"
                "; } else {\n"
                "$__cond_f_%p:\n"
                , this, this);

        altCode.getScope()->setParent(scope);
        TRY_B(altCode.compile(ofile, rtype, prog));

        fprintf(ofile,
                "$__cond_end_%p:\n"
                "; }\n"
                , this);

        return false;
    }

    bool Statement::VMIN(VarDecl, compile)(FILE *ofile, Scope *scope, TypeSpec, const Program *prog) {
        TRY_B(scope->addVar(&var));

        // TODO: More precise logging (type, etc.)
        fprintf(ofile,
                "; var %.*s\n",
                var.getName()->getLength(), var.getName()->getStr());

        TypeSpec::Mask mask = expr.deduceType(TypeSpec::VoidMask | var.getType().getMask(), scope, prog);

        TRY_B(expr.compile(ofile, scope, prog));  // TODO: Expr again

        if (mask != TypeSpec::VoidMask) {
            fprintf(ofile,
                    "; = \n"
                    "pop ");
            TRY_B(var.compile(ofile, scope));
            fprintf(ofile, "\n");
        }

        return false;
    }

    bool Statement::VMIN(Expr, compile)(FILE *ofile, Scope *scope, TypeSpec, const Program *prog) {
        TypeSpec::Mask mask = expr.deduceType(TypeSpec::AllMask, scope, prog);

        TRY_B(expr.compile(ofile, scope, prog));  // Ambiguousness of the mask is checked inside

        if (mask != TypeSpec::VoidMask) {
            fprintf(ofile, "popv\n");
        }

        return false;
    }

    bool Statement::VMIN(Empty, compile)(FILE *, Scope *, TypeSpec, const Program *) {
        return false;
    }


    void Statement::VMIN(Compound, reconstruct)(FILE *ofile, unsigned indent) const {
        fprintf(ofile, "{\n");
        code.reconstruct(ofile, indent + 1);

        for (unsigned i = 0; i < indent; ++i)
            fprintf(ofile, "    ");

        fprintf(ofile, "}");
    }

    void Statement::VMIN(Return, reconstruct)(FILE *ofile, unsigned) const {
        fprintf(ofile, "ret");

        if (!expr.isVoid()) {
            fprintf(ofile, " ");
            expr.reconstruct(ofile);
        }

        fprintf(ofile, ";");
    }

    void Statement::VMIN(Loop, reconstruct)(FILE *ofile, unsigned indent) const {
        fprintf(ofile, "while ");

        expr.reconstruct(ofile);

        fprintf(ofile, " {\n");

        code.reconstruct(ofile, indent + 1);

        for (unsigned i = 0; i < indent; ++i)
            fprintf(ofile, "    ");

        fprintf(ofile, "}");
    }

    void Statement::VMIN(Cond, reconstruct)(FILE *ofile, unsigned indent) const {
        fprintf(ofile, "if ");

        expr.reconstruct(ofile);

        fprintf(ofile, " {\n");

        code.reconstruct(ofile, indent + 1);

        for (unsigned i = 0; i < indent; ++i)
            fprintf(ofile, "    ");

        fprintf(ofile, "} else {\n");

        altCode.reconstruct(ofile, indent + 1);

        for (unsigned i = 0; i < indent; ++i)
            fprintf(ofile, "    ");

        fprintf(ofile, "}");
    }

    void Statement::VMIN(VarDecl, reconstruct)(FILE *ofile, unsigned) const {
        fprintf(ofile, "var ");
        var.getType().reconstruct(ofile);
        fprintf(ofile, "%.*s", var.getName()->getLength(), var.getName()->getStr());

        if (!expr.isVoid()) {
            fprintf(ofile, " = ");

            expr.reconstruct(ofile);
        }

        fprintf(ofile, ";");
    }

    void Statement::VMIN(Expr, reconstruct)(FILE *ofile, unsigned) const {
        expr.reconstruct(ofile);
        fprintf(ofile, ";");
    }

    void Statement::VMIN(Empty, reconstruct)(FILE *ofile, unsigned) const {
        fprintf(ofile, ";");
    }

    #define DEF_TYPE(NAME) \
        VTYPE_DEF(NAME, Statement) = { \
            Statement::VMIN(NAME, dtor), \
            Statement::VMIN(NAME, compile), \
            Statement::VMIN(NAME, reconstruct), \
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

    void Function::popArg() {
        args.pop();
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

        // TODO: Maybe do it a bit less manually, at the cost of speed

        uint32_t offset = code.getScope()->getFrameSize();

        for (unsigned i = args.getSize() - 1; i != (unsigned)-1; --i) {
            offset -= args[i].getType().getSize();

            assert(offset < (uint32_t)-1000);  // Checks that it isn't negative

            fprintf(ofile, "pop ");
            TRY_B(args[i].getType().compile(ofile));
            fprintf(ofile, "[rz+%u]\n", offset);
        }

        code.getScope()->setParent(nullptr);
        TRY_B(code.compile(ofile, rtype, prog));

        fprintf(ofile, "ret  ; Force end of $%.*s\n\n", name->getLength(), name->getStr());

        return false;
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

    const Vector<Var> *Function::getArgs() const {
        return &args;
    }

    void Function::reconstruct(FILE *ofile) const {
        fprintf(ofile, "def %.*s(", name->getLength(), name->getStr());

        const Token *curName = nullptr;

        if (args.getSize() >= 1) {
            curName = args[0].getName();

            fprintf(ofile, "%.*s", curName->getLength(), curName->getStr());
        }

        for (unsigned i = 1; i < args.getSize(); ++i) {
            curName = args[i].getName();

            fprintf(ofile, ", %.*s", curName->getLength(), curName->getStr());
        }

        fprintf(ofile, ") {\n");

        code.reconstruct(ofile, 1);

        fprintf(ofile, "}\n\n");
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

    void Program::popFunction() {
        functions.pop();
    }

    bool Program::compile(FILE* ofile) {
        fprintf(ofile,
                "; === [ ALFC ver. NULL ] ===\n\n"
                "; Entrypoint + loader:\n"
                "$main:\n"
                "    push dwl:4096\n"
                "    pop dwl:rz  ; Function frame counter\n"
                "    call dwl:$__func_main\n"  // TODO: If we decide to actually have void as zero, popv needs to be placed here
                "    end\n"
                "\n");

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

    void Program::reconstruct(FILE *ofile) const {
        for (unsigned i = 0; i < functions.getSize(); ++i) {
            functions[i].reconstruct(ofile);
        }
    }

}

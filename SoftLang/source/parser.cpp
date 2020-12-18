#include "parser.h"

#include <cassert>


namespace SoftLang {

    bool Parser::ctor() {
        TRY_B(lexer.ctor());

        return false;
    }

    bool Parser::ctor(const FileBuf *src) {
        TRY_B(lexer.ctor(src));

        return false;
    }

    void Parser::dtor() {
        lexer.dtor();
    }

    inline const Token *Parser::cur() const {
        return lexer.cur();
    }

    inline const Token *Parser::peek(int delta) const {
        return lexer.peek(delta);
    }

    inline const Token *Parser::next() {
        return lexer.next();
    }

    inline const Token *Parser::prev() {
        return lexer.prev();
    }

    inline unsigned Parser::backup() const {
        return lexer.backup();
    }

    inline void Parser::restore(unsigned saved) {
        lexer.restore(saved);
    }

    #define P_TRY(STMT, ONSUCC, ONFAIL) {   \
        switch (STMT) {                     \
        case ERR_PARSER_OK:                 \
            ONSUCC;                         \
            break;                          \
        case ERR_PARSER_SYS:                \
            return ERR_PARSER_SYS;          \
        case ERR_PARSER_LEX:                \
            assert(false);                  \
            return ERR_PARSER_LEX;          \
        case ERR_PARSER_SYNTAX:             \
            ONFAIL;                         \
            break;                          \
        default:                            \
            assert(false);                  \
            break;                          \
        }                                   \
    }

    // TODO: Maybe goto error_sys?
    #define P_TRYSYS(STMT) {                \
        if (STMT) {                         \
            return ERR_PARSER_SYS;          \
        }                                   \
    }

    #define P_OK() \
        return ERR_PARSER_OK;

    #define P_BAD() \
        return ERR_PARSER_SYNTAX;

    #define P_REQ_KWD(KWD)                                                                              \
        if (cur()->getKwd() != Token::KWD_##KWD) { /* Will automatically account for non-kwd tokens */  \
            goto error;                                                                                 \
        }                                                                                               \
        next();

    #define P_REQ_PUNCT(PUNCT)                                      \
        if (cur()->getPunct() != Token::PUNCT_##PUNCT) { /* Same */ \
            goto error;                                             \
        }                                                           \
        next();

    #define P_REQ_NONTERM(NONTERM, ...) \
        P_TRY(parse_##NONTERM(__VA_ARGS__), , goto error)

    Parser::Error_e Parser::parse(Program *prog) {
        assert(prog);

        P_TRYSYS(lexer.parse());

        if (lexer.getError()) {
            return ERR_PARSER_LEX;
        }

        P_TRYSYS(!lexer.getEnd());

        P_REQ_NONTERM(FUNC_DEFS, prog);

        if (!cur()->isEnd())
            goto error;

        P_OK();

    error:
        prog->dtor();  // Most of the things are cleaned up here (in a recursive destructor), so
                       // functionwise error's should only care about destroying the objects they
        P_BAD();       // had created within their respective functions
    }

    Parser::Error_e Parser::parse_FUNC_DEFS(Program *prog) {
        bool repeat = true;

        P_TRYSYS(prog->ctor());

        while (repeat) {
            P_TRY(parse_FUNC_DEF(prog), , repeat = false);
        }

        P_OK();
    }

    Parser::Error_e Parser::parse_FUNC_DEF(Program *prog) {
        Function *func = nullptr;
        TypeSpec ts{};
        const Token *name = nullptr;
        Code *code = nullptr;

        P_TRYSYS(prog->makeFunction(&func));

        P_REQ_KWD(DEF);

        P_TRY(parse_TYPESPEC(&ts),
              P_REQ_PUNCT(COLON),
              ts.dtor(); ts.ctor(TypeSpec::T_VOID));

        P_REQ_NONTERM(FUNC, &name);
        P_REQ_PUNCT(LPAR);

        func->ctor(ts, name);

        P_REQ_NONTERM(FUNC_ARGS_DEF, func);
        P_REQ_PUNCT(RPAR);

        P_TRYSYS(func->makeCode(&code));

        P_REQ_NONTERM(COMPOUND_STMT, code);
        P_OK();

    error:
        ts.dtor();

        P_BAD();
    }

    Parser::Error_e Parser::parse_FUNC_ARGS_DEF(Function *func) {
        Var *arg = nullptr;

        P_TRYSYS(func->makeArg(&arg));
        P_TRY(parse_FUNC_ARG_DEF(arg), , P_OK());

        while (cur()->getPunct() == Token::PUNCT_COMMA) {
            next();

            P_TRYSYS(func->makeArg(&arg));
            P_REQ_NONTERM(FUNC_ARG_DEF, arg);
        }

        P_OK();

    error:
        P_BAD();
    }

    Parser::Error_e Parser::parse_FUNC_ARG_DEF(Var *arg) {
        TypeSpec ts{};

        P_REQ_NONTERM(TYPESPEC, &ts);
        P_REQ_PUNCT(COLON);

        if (!cur()->isName()) {
            goto error;
        }

        P_TRYSYS(arg->ctor(ts, next()));

        P_OK();

    error:
        ts.dtor();

        P_BAD();
    }

    Parser::Error_e Parser::parse_TYPESPEC(TypeSpec *ts) {
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wswitch-enum"

        switch (next()->getKwd()) {
        case Token::KWD_DBL:
            ts->ctor(TypeSpec::T_DBL);

            P_OK();

        case Token::KWD_INT4:
            ts->ctor(TypeSpec::T_INT4);

            P_OK();

        case Token::KWD_INT8:
            ts->ctor(TypeSpec::T_INT8);

            P_OK();

        default:
            prev();
            P_BAD();
        }

        #pragma GCC diagnostic pop
    }

    Parser::Error_e Parser::parse_STMTS(Code *code) {
        P_TRYSYS(code->ctor());

        while (true) {
            Statement *stmt = nullptr;
            P_TRYSYS(code->makeStatement(&stmt));

            P_TRY(parse_STMT(stmt), , P_OK());

            code->simplifyLastEmpty();
        }
    }

    Parser::Error_e Parser::parse_STMT(Statement *stmt) {
        unsigned saved = backup();

        P_TRY(parse_COMPOUND_STMT(stmt),  P_OK(), restore(saved));
        P_TRY(parse_RETURN_STMT(stmt),    P_OK(), restore(saved));
        P_TRY(parse_LOOP_STMT(stmt),      P_OK(), restore(saved));
        P_TRY(parse_COND_STMT(stmt),      P_OK(), restore(saved));
        P_TRY(parse_VARDECL_STMT(stmt),   P_OK(), restore(saved));
        P_TRY(parse_EXPR_STMT(stmt),      P_OK(), restore(saved));

        P_REQ_PUNCT(SEMI);
        P_TRYSYS(stmt->ctorEmpty());

        P_OK();

    error:
        P_BAD();
    }

    Parser::Error_e Parser::parse_COMPOUND_STMT(Code *code) {
        P_REQ_PUNCT(LBRACE);

        P_REQ_NONTERM(STMTS, code);

        P_REQ_PUNCT(RBRACE);

        P_OK();

    error:
        P_BAD();
    }

    Parser::Error_e Parser::parse_COMPOUND_STMT(Statement *stmt) {
        Code *code = nullptr;

        P_TRYSYS(stmt->ctorCompound());
        P_TRYSYS(stmt->makeCode(&code));

        P_REQ_NONTERM(COMPOUND_STMT, code);

    error:
        stmt->dtor();

        P_BAD();
    }

    Parser::Error_e Parser::parse_RETURN_STMT(Statement *stmt) {
        Expression *expr = nullptr;
        unsigned saved = 0;

        P_TRYSYS(stmt->ctorReturn());
        P_TRYSYS(stmt->makeExpr(&expr));

        P_REQ_KWD(RET);

        saved = backup();
        P_TRY(parse_EXPR(expr),
              ,
              restore(saved); expr->dtor(); expr->ctorVoid());

        P_REQ_PUNCT(SEMI);

        P_OK();

    error:
        stmt->dtor();

        P_BAD();
    }

    Parser::Error_e Parser::parse_LOOP_STMT(Statement *stmt) {
        Expression *cond = nullptr;
        Code       *code = nullptr;

        P_TRYSYS(stmt->ctorLoop());
        P_TRYSYS(stmt->makeExpr(&cond));
        P_TRYSYS(stmt->makeCode(&code));

        P_REQ_KWD(WHILE);

        P_REQ_NONTERM(EXPR, cond);

        P_REQ_NONTERM(COMPOUND_STMT, code);

        P_OK();

    error:
        stmt->dtor();

        P_BAD();
    }

    Parser::Error_e Parser::parse_COND_STMT(Statement *stmt) {
        Expression *cond    = nullptr;
        Code       *code    = nullptr;
        Code       *altCode = nullptr;

        P_TRYSYS(stmt->ctorCond());
        P_TRYSYS(stmt->makeExpr(&cond));
        P_TRYSYS(stmt->makeCode(&code));
        P_TRYSYS(stmt->makeAltCode(&altCode));

        P_REQ_KWD(IF);

        P_REQ_NONTERM(EXPR, cond);

        P_REQ_NONTERM(COMPOUND_STMT, code);

        if (cur()->getKwd() == Token::KWD_ELSE) {
            next();

            P_REQ_NONTERM(COMPOUND_STMT, altCode);
        } else {
            altCode->ctor();
        }

        P_OK();

    error:
        stmt->dtor();

        P_BAD();
    }

    Parser::Error_e Parser::parse_VARDECL_STMT(Statement *stmt) {
        Var *var = nullptr;
        Expression *expr = nullptr;  // TODO: Make sure that fields necessary for every virtual type of Stmt don't overlap

        P_TRYSYS(stmt->ctorVarDecl());
        P_TRYSYS(stmt->makeVar(&var));
        P_TRYSYS(stmt->makeExpr(&expr));

        P_REQ_NONTERM(VARDECL, var);

        if (cur()->getPunct() == Token::PUNCT_EQ) {
            next();

            P_REQ_NONTERM(EXPR, expr);
        } else {
            expr->ctorVoid();
        }

        P_REQ_PUNCT(SEMI);

        P_OK();

    error:
        stmt->dtor();

        P_BAD();
    }

    Parser::Error_e Parser::parse_EXPR_STMT(Statement *stmt) {
        Expression *expr = nullptr;
        P_TRYSYS(stmt->ctorExpr());
        P_TRYSYS(stmt->makeExpr(&expr));

        P_REQ_NONTERM(EXPR, expr);

        P_REQ_PUNCT(SEMI);

        P_OK();

    error:
        stmt->dtor();

        P_BAD();
    }

    Parser::Error_e Parser::parse_EXPR(Expression *expr) {
        unsigned saved = backup();

        P_TRY(parse_ASGN_EXPR(expr),  P_OK(), restore(saved));
        //P_TRY(parse_OR_EXPR(expr),    P_OK(), restore(saved));
        P_TRY(parse_CMP_EXPR(expr),   P_OK(), restore(saved));

        P_BAD();
    }

    Parser::Error_e Parser::parse_ASGN_EXPR(Expression *expr) {
        Expression *var = nullptr;
        Expression *child = nullptr;
        P_TRYSYS(expr->ctorAsgn());
        P_TRYSYS(expr->makeChild(&var));
        P_TRYSYS(expr->makeChild(&child));

        P_REQ_NONTERM(VAR_EXPR, var);

        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wswitch-enum"

        switch (next()->getPunct()) {
        case Token::PUNCT_EQ:
            expr->setAsgnMode(Expression::AM_EQ);
            break;

        case Token::PUNCT_ADDEQ:
            expr->setAsgnMode(Expression::AM_ADDEQ);
            break;

        case Token::PUNCT_SUBEQ:
            expr->setAsgnMode(Expression::AM_SUBEQ);
            break;

        case Token::PUNCT_MULEQ:
            expr->setAsgnMode(Expression::AM_MULEQ);
            break;

        case Token::PUNCT_DIVEQ:
            expr->setAsgnMode(Expression::AM_DIVEQ);
            break;

        case Token::PUNCT_MODEQ:
            expr->setAsgnMode(Expression::AM_MODEQ);
            break;

        default:
            prev();

            goto error;
        }

        #pragma GCC diagnostic pop

        P_REQ_NONTERM(EXPR, child);

        P_OK();

    error:
        expr->dtor();

        P_BAD();
    }

    /*Parser::Error_e Parser::parse_OR_EXPR() {
        bool repeat = true;

        while (repeat) {
            P_REQ_NONTERM(AND_EXPR);

            repeat = next()->getPunct() == Token::PUNCT_OR;
        }
        prev();

        P_OK();
    }

    Parser::Error_e Parser::parse_AND_EXPR() {
        bool repeat = true;

        while (repeat) {
            P_REQ_NONTERM(CMP_EXPR);

            repeat = next()->getPunct() == Token::PUNCT_AND;
        }
        prev();

        P_OK();
    }*/

    Parser::Error_e Parser::parse_CMP_EXPR(Expression *expr) {
        Expression *child = nullptr;

        P_TRYSYS(expr->ctorPolyOp());

        bool repeat = true;
        unsigned ind = 0;

        while (repeat) {
            P_TRYSYS(expr->makeChild(&child));
            P_REQ_NONTERM(ADD_EXPR, child);

            #pragma GCC diagnostic push
            #pragma GCC diagnostic ignored "-Wswitch-enum"

            switch (next()->getPunct()) {
            case Token::PUNCT_EQEQ:
                P_TRYSYS(expr->setOp(ind, Expression::OP_EQ));
                break;

            case Token::PUNCT_NEQ:
                P_TRYSYS(expr->setOp(ind, Expression::OP_NEQ));
                break;

            case Token::PUNCT_GEQ:
                P_TRYSYS(expr->setOp(ind, Expression::OP_GEQ));
                break;

            case Token::PUNCT_LEQ:
                P_TRYSYS(expr->setOp(ind, Expression::OP_LEQ));
                break;

            case Token::PUNCT_GT:
                P_TRYSYS(expr->setOp(ind, Expression::OP_GT));
                break;

            case Token::PUNCT_LT:
                P_TRYSYS(expr->setOp(ind, Expression::OP_LT));
                break;

            default:
                repeat = false;
                prev();
                break;
            }

            ind++;

            #pragma GCC diagnostic pop
        }

        P_OK();

    error:
        expr->dtor();

        P_BAD();
    }

    Parser::Error_e Parser::parse_ADD_EXPR(Expression *expr) {
        Expression *child = nullptr;

        P_TRYSYS(expr->ctorPolyOp());

        bool repeat = true;
        unsigned ind = 0;

        while (repeat) {
            P_TRYSYS(expr->makeChild(&child));
            P_REQ_NONTERM(MUL_EXPR, child);

            #pragma GCC diagnostic push
            #pragma GCC diagnostic ignored "-Wswitch-enum"

            switch (next()->getPunct()) {
            case Token::PUNCT_ADD:
                P_TRYSYS(expr->setOp(ind, Expression::OP_ADD));
                break;

            case Token::PUNCT_SUB:
                P_TRYSYS(expr->setOp(ind, Expression::OP_SUB));
                break;

            default:
                repeat = false;
                prev();
                break;
            }

            ind++;

            #pragma GCC diagnostic pop
        }

        P_OK();

    error:
        expr->dtor();

        P_BAD();
    }

    Parser::Error_e Parser::parse_MUL_EXPR(Expression *expr) {
        Expression *child = nullptr;

        P_TRYSYS(expr->ctorPolyOp());

        bool repeat = true;
        unsigned ind = 0;

        while (repeat) {
            P_TRYSYS(expr->makeChild(&child));
            P_REQ_NONTERM(UNARY_EXPR, child);

            #pragma GCC diagnostic push
            #pragma GCC diagnostic ignored "-Wswitch-enum"

            switch (next()->getPunct()) {
            case Token::PUNCT_MUL:
                P_TRYSYS(expr->setOp(ind, Expression::OP_MUL));
                break;

            case Token::PUNCT_DIV:
                P_TRYSYS(expr->setOp(ind, Expression::OP_DIV));
                break;

            case Token::PUNCT_MOD:
                P_TRYSYS(expr->setOp(ind, Expression::OP_MOD));
                break;

            default:
                repeat = false;
                prev();
                break;
            }

            ind++;

            #pragma GCC diagnostic pop
        }

        P_OK();

    error:
        expr->dtor();

        P_BAD();
    }

    Parser::Error_e Parser::parse_UNARY_EXPR(Expression *expr) {  // TODO: Finish semantics
        TypeSpec ts{};
        Expression *child = nullptr;
        bool isTypeCast = false;
        unsigned saved = backup();

        if (cur()->getPunct() == Token::PUNCT_SUB) {
            next();

            P_TRYSYS(expr->ctorNeg());
            P_TRYSYS(expr->makeChild(&child));

            P_REQ_NONTERM(UNARY_EXPR, child);

            P_OK();
        }

        if (cur()->getPunct() == Token::PUNCT_ADD) {
            next();

            P_REQ_NONTERM(UNARY_EXPR, expr);

            P_OK();
        }

        if (cur()->getPunct() == Token::PUNCT_LPAR) {
            next();

            P_REQ_NONTERM(EXPR, expr);

            P_REQ_PUNCT(RPAR);

            P_OK();
        }

        // It's a single-lexem nonterminal, so we don't need to back up the position
        P_TRY(parse_TYPESPEC(&ts), isTypeCast = true, );

        if (isTypeCast) {
            P_TRYSYS(expr->ctorCast(ts));
            P_TRYSYS(expr->makeChild(&child));

            P_REQ_PUNCT(COLON);
            P_REQ_NONTERM(UNARY_EXPR, child);

            P_OK();
        }

        if (cur()->isNum()) {
            P_TRYSYS(expr->ctorNum(next()));

            P_OK();
        }

        P_TRY(parse_FUNC_CALL(expr), P_OK(), restore(saved));

        P_REQ_NONTERM(VAR_EXPR, expr);

        P_OK();

    error:
        expr->dtor();
        ts.dtor();

        P_BAD();
    }

    Parser::Error_e Parser::parse_VAR_EXPR(Expression *expr) {
        const Token *name = nullptr;

        P_REQ_NONTERM(VAR, &name);

        P_TRYSYS(expr->ctorVarRef(name));

        P_OK();

    error:
        P_BAD();
    }

    Parser::Error_e Parser::parse_VAR(const Token **name) {
        if (cur()->isName()) {
            *name = next();

            P_OK();
        }

        P_BAD();
    }

    Parser::Error_e Parser::parse_VARDECL(Var *var) {
        TypeSpec ts{};
        const Token *name = nullptr;

        P_REQ_KWD(VAR);

        P_REQ_NONTERM(TYPESPEC, &ts);

        P_REQ_PUNCT(COLON);

        P_REQ_NONTERM(VAR, &name);

        P_TRYSYS(var->ctor(ts, name));

        P_OK();

    error:
        ts.dtor();

        P_BAD();
    }

    // Identical to VAR, but separated for clearance reasons
    Parser::Error_e Parser::parse_FUNC(const Token **name) {
        if (cur()->isName()) {
            *name = next();

            P_OK();
        }

        P_BAD();
    }

    Parser::Error_e Parser::parse_FUNC_CALL(Expression *expr) {
        const Token *name = nullptr;

        P_REQ_NONTERM(FUNC, &name);

        P_TRYSYS(expr->ctorFuncCall(name));

        P_REQ_PUNCT(LPAR);

        P_REQ_NONTERM(FUNC_ARGS, expr);

        P_REQ_PUNCT(RPAR);

        P_OK();

    error:
        P_BAD();
    }

    Parser::Error_e Parser::parse_FUNC_ARGS(Expression *expr) {
        Expression *child = nullptr;

        bool repeat = true;

        while (repeat) {
            P_TRYSYS(expr->makeChild(&child));
            P_REQ_NONTERM(EXPR, child);

            repeat = next()->getPunct() == Token::PUNCT_COMMA;
        }
        prev();

        P_OK();

    error:
        P_BAD();
    }

    #undef P_TRY
    #undef P_TRYSYS
    #undef P_OK
    #undef P_BAD
    #undef P_REQ_KWD
    #undef P_REQ_PUNCT
    #undef P_REQ_NONTERM

}

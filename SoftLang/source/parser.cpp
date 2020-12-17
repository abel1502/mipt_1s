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

    #define P_OK() \
        return ERR_PARSER_OK;

    #define P_BAD() \
        return ERR_PARSER_SYNTAX;

    #define P_REQ_KWD(KWD)                                                                              \
        if (cur()->getKwd() != Token::KWD_##KWD) { /* Will automatically account for non-kwd tokens */  \
            P_BAD();                                                                                    \
        }                                                                                               \
        next();

    #define P_REQ_PUNCT(PUNCT)                                      \
        if (cur()->getPunct() != Token::PUNCT_##PUNCT) { /* Same */ \
            P_BAD();                                                \
        }                                                           \
        next();

    #define P_REQ_NONTERM(NONTERM, ...) \
        P_TRY(parse_##NONTERM(__VA_ARGS__), , P_BAD())

    Parser::Error_e Parser::parse() {
        if (lexer.parse()) {
            return ERR_PARSER_SYS;
        }

        if (lexer.getError()) {
            return ERR_PARSER_LEX;
        }

        if (!lexer.getEnd()) {
            return ERR_PARSER_SYS;
        }

        P_REQ_NONTERM(FUNC_DEFS);

        if (!cur()->isEnd()) {
            P_BAD();
        }

        P_OK();
    }

    Parser::Error_e Parser::parse_FUNC_DEFS() {
        while (true) {
            P_TRY(parse_FUNC_DEF(), , P_OK());
        }
    }

    Parser::Error_e Parser::parse_FUNC_DEF() {
        P_REQ_KWD(DEF);
        P_REQ_NONTERM(TYPESPEC);
        P_REQ_PUNCT(COLON);

        if (!cur()->isName()) {
            P_BAD();
        }
        next();

        P_REQ_PUNCT(LPAR);
        P_REQ_NONTERM(FUNC_ARGS_DEF);
        P_REQ_PUNCT(RPAR);
        P_REQ_NONTERM(COMPOUND_STMT);

        P_OK();
    }

    Parser::Error_e Parser::parse_FUNC_ARGS_DEF() {
        P_TRY(parse_FUNC_ARG_DEF(), , P_OK());

        while (cur()->getPunct() == Token::PUNCT_COMMA) {
            next();

            P_REQ_NONTERM(FUNC_ARG_DEF);
        }

        P_OK();
    }

    Parser::Error_e Parser::parse_FUNC_ARG_DEF() {
        P_REQ_NONTERM(TYPESPEC);
        P_REQ_PUNCT(COLON);

        if (!cur()->isName()) {
            P_BAD();
        }
        next();

        P_OK();
    }

    Parser::Error_e Parser::parse_TYPESPEC() {
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wswitch-enum"

        switch (next()->getKwd()) {
        case Token::KWD_DBL:
            P_OK();
        case Token::KWD_INT4:
            P_OK();
        case Token::KWD_INT8:
            P_OK();
        default:
            prev();
            P_BAD();
        }

        #pragma GCC diagnostic pop
    }

    Parser::Error_e Parser::parse_STMTS() {
        while (true) {
            P_TRY(parse_STMT(), , P_OK());
        }
    }

    Parser::Error_e Parser::parse_STMT() {
        unsigned saved = backup();

        P_TRY(parse_COMPOUND_STMT(),  P_OK(), restore(saved));
        P_TRY(parse_RETURN_STMT(),    P_OK(), restore(saved));
        P_TRY(parse_LOOP_STMT(),      P_OK(), restore(saved));
        P_TRY(parse_COND_STMT(),      P_OK(), restore(saved));
        P_TRY(parse_VARDECT_STMT(),   P_OK(), restore(saved));
        P_TRY(parse_EXPR_STMT(),      P_OK(), restore(saved));

        P_REQ_PUNCT(SEMI);

        P_OK();
    }

    Parser::Error_e Parser::parse_COMPOUND_STMT() {
        P_REQ_PUNCT(LBRACE);

        P_REQ_NONTERM(STMTS);

        P_REQ_PUNCT(RBRACE);

        P_OK();
    }

    Parser::Error_e Parser::parse_RETURN_STMT() {
        P_REQ_KWD(RET);

        unsigned saved = backup();
        P_TRY(parse_EXPR(), , restore(saved));

        P_REQ_PUNCT(SEMI);

        P_OK();
    }

    Parser::Error_e Parser::parse_LOOP_STMT() {
        P_REQ_KWD(WHILE);

        P_REQ_NONTERM(EXPR);

        P_REQ_NONTERM(COMPOUND_STMT);

        P_OK();
    }

    Parser::Error_e Parser::parse_COND_STMT() {
        P_REQ_KWD(IF);

        P_REQ_NONTERM(EXPR);

        P_REQ_NONTERM(COMPOUND_STMT);

        P_OK();
    }

    Parser::Error_e Parser::parse_VARDECL_STMT() {
        P_REQ_NONTERM(VARDECL);

        if (cur()->getPunct() == Token::PUNCT_EQ) {
            next();

            P_REQ_NONTERM(EXPR);
        }

        P_REQ_PUNCT(SEMI);

        P_OK();
    }

    Parser::Error_e Parser::parse_EXPR_STMT() {
        P_REQ_NONTERM(EXPR);

        P_REQ_PUNCT(SEMI);

        P_OK();
    }

    Parser::Error_e Parser::parse_EXPR() {
        unsigned saved = backup();

        P_TRY(parse_ASGN_EXPR(),  P_OK(), restore(saved));
        P_TRY(parse_OR_EXPR(),    P_OK(), restore(saved));

        P_BAD();
    }

    Parser::Error_e Parser::parse_ASGN_EXPR() {
        P_REQ_NONTERM(VAR);

        switch (next()->getPunct()) {
        case Token::PUNCT_EQ:
            break;
        case Token::PUNCT_ADDEQ:
            break;
        case Token::PUNCT_SUBEQ:
            break;
        case Token::PUNCT_MULEQ:
            break;
        case Token::PUNCT_DIVEQ:
            break;
        case Token::PUNCT_MODEQ:
            break;
        default:
            prev();
            P_BAD();
        }

        P_REQ_NONTERM(EXPR);

        P_OK();
    }

    Parser::Error_e Parser::parse_OR_EXPR() {
        bool reapeat = true;

        while (repeat) {
            P_REQ_NONTERM(AND_EXPR);

            repeat = next()->getPunct() == Token::PUNCT_OR;
        }
        prev();

        P_OK();
    }

    Parser::Error_e Parser::parse_AND_EXPR() {
        bool reapeat = true;

        while (repeat) {
            P_REQ_NONTERM(CMP_EXPR);

            repeat = next()->getPunct() == Token::PUNCT_AND;
        }
        prev();

        P_OK();
    }

    Parser::Error_e Parser::parse_CMP_EXPR() {
        bool reapeat = true;

        while (repeat) {
            P_REQ_NONTERM(ADD_EXPR);

            switch (next()->getPunct()) {
            case Token::PUNCT_EQEQ:
                break;
            case Token::PUNCT_NEQ:
                break;
            case Token::PUNCT_GEQ:
                break;
            case Token::PUNCT_LEQ:
                break;
            case Token::PUNCT_GT:
                break;
            case Token::PUNCT_LT:
                break;
            default:
                repeat = false;
                prev();
                break;
            }
        }

        P_OK();
    }

    Parser::Error_e Parser::parse_ADD_EXPR() {
    }

    Parser::Error_e Parser::parse_MUL_EXPR();

    Parser::Error_e Parser::parse_UNARY_EXPR();

    Parser::Error_e Parser::parse_VAR();

    Parser::Error_e Parser::parse_VARDECL();

    Parser::Error_e Parser::parse_FUNC_CALL();

    Parser::Error_e Parser::parse_FUNC_ARGS();

    Parser::Error_e Parser::parse_FUNC_ARG();

    #undef P_TRY
    #undef P_OK
    #undef P_BAD
    #undef P_REQ_KWD
    #undef P_REQ_PUNCT
    #undef P_REQ_NONTERM

}

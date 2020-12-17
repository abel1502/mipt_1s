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

        TRY(parse_FUNC_DEFS());

        // TODO
    }

    Parser::Error_e Parser::parse_FUNC_DEFS() {
        //
    }

    Parser::Error_e Parser::parse_FUNC_DEF();

    Parser::Error_e Parser::parse_FUNC_ARGS_DEF();

    Parser::Error_e Parser::parse_FUNC_ARG_DEF();

    Parser::Error_e Parser::parse_TYPESPEC();

    Parser::Error_e Parser::parse_STMTS();

    Parser::Error_e Parser::parse_STMT();

    Parser::Error_e Parser::parse_COMPOUND_STMT();

    Parser::Error_e Parser::parse_RETURN_STMT();

    Parser::Error_e Parser::parse_LOOP_STMT();

    Parser::Error_e Parser::parse_COND_STMT();

    Parser::Error_e Parser::parse_VARDECL_STMT();

    Parser::Error_e Parser::parse_EXPR_STMT();

    Parser::Error_e Parser::parse_EXPR();

    Parser::Error_e Parser::parse_ASGN_EXPR();

    Parser::Error_e Parser::parse_OR_EXPR();

    Parser::Error_e Parser::parse_AND_EXPR();

    Parser::Error_e Parser::parse_CMP_EXPR();

    Parser::Error_e Parser::parse_ADD_EXPR();

    Parser::Error_e Parser::parse_MUL_EXPR();

    Parser::Error_e Parser::parse_UNARY_EXPR();

    Parser::Error_e Parser::parse_VAR();

    Parser::Error_e Parser::parse_VARDECL();

    Parser::Error_e Parser::parse_FUNC_CALL();

    Parser::Error_e Parser::parse_FUNC_ARGS();

    Parser::Error_e Parser::parse_FUNC_ARG();

}

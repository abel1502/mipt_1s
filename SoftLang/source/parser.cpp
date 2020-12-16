#include "parser.h"


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

        // TODO
    }

}

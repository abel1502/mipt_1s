#ifndef PARSER_H
#define PARSER_H

#include "general.h"
#include "filebuf.h"
#include "lexer.h"


namespace SoftLang {

    class Parser {
    public:

        enum Error_e {
            ERR_PARSER_OK = 0,
            ERR_PARSER_SYS,
            ERR_PARSER_LEX,
            ERR_PARSER_SYNTAX
        };

        FACTORIES(Parser)

        bool ctor();

        bool ctor(const FileBuf *src);

        void dtor();

        Error_e parse();

        Error_e parse_FUNC_DEFS();

        Error_e parse_FUNC_DEF();

        Error_e parse_FUNC_ARGS_DEF();

        Error_e parse_FUNC_ARG_DEF();

        Error_e parse_TYPESPEC();

        Error_e parse_STMTS();

        Error_e parse_STMT();

        Error_e parse_COMPOUND_STMT();

        Error_e parse_RETURN_STMT();

        Error_e parse_LOOP_STMT();

        Error_e parse_COND_STMT();

        Error_e parse_VARDECL_STMT();

        Error_e parse_EXPR_STMT();

        Error_e parse_EXPR();

        Error_e parse_ASGN_EXPR();

        Error_e parse_OR_EXPR();

        Error_e parse_AND_EXPR();

        Error_e parse_CMP_EXPR();

        Error_e parse_ADD_EXPR();

        Error_e parse_MUL_EXPR();

        Error_e parse_UNARY_EXPR();

        Error_e parse_VAR();

        Error_e parse_VARDECL();

        Error_e parse_FUNC_CALL();

        Error_e parse_FUNC_ARGS();

        Error_e parse_FUNC_ARG();

    private:
        Lexer lexer;

    };

}


#endif // PARSER_H

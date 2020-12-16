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

    private:
        Lexer lexer;

    };

}


#endif // PARSER_H

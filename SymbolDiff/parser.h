#ifndef PARSER_H
#define PARSER_H

#include "general.h"
#include "expr.h"

#include <cstdio>


namespace SymbolDiff {

    class ExprNode;


    class Parser {
    public:
        FACTORIES(Parser)

        Parser *ctor();

        void dtor();

        void feed(FILE *ifile);

        void feed(const char *src);

        bool isEof();

        /**
         * This parser automatically skips all whitespace characters, so to identify them
         * you may use the following two functions.They return true if when transitioning to
         */

        bool wasSpace();

        bool wasLine();

        char cur();

        /**
         * The following two methods return, essentially, cur().
         */

        char next();

        char prev();

        void logErr();

        bool isValid();

        /**
         * True means error
         */

        bool parse(ExprNode *dest);

        bool parseInt(long long *dest);

        bool parseBinOp(int *dest);

        bool parseUnOp(int *dest);

        bool parseBinOpNode(ExprNode *dest);

        bool parseUnOpNode(ExprNode *dest);

        bool parseExprNode(ExprNode *dest);

    private:
        unsigned size;
        char *buf;

        unsigned pos;

        unsigned errPos;

        bool fSpace;
        bool fLine;

        static unsigned fsize(FILE *ifile);
    };

}


#endif // PARSER_H

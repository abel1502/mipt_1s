#ifndef LEXER_H
#define LEXER_H

#include "general.h"
#include "filebuf.h"

#include <cstdlib>
#include <cstdio>


namespace SoftLang {

    class Token {
    public:

        enum Type_e {
            TOK_ERROR = 0,  // A special value to indicate errors, shouldn't be present in valid results
            TOK_END,
            TOK_NAME,
            TOK_NUM,
            TOK_KWD,
            TOK_PUNCT
        };

        enum Kwd_e {
            KWD_ERROR = 0,  // -"-

            #define DEF_KWD(NAME, STR)  KWD_##NAME,
            #define DEF_PUNCT(NAME, STR)

            #include "tokens.dsl.h"

            #undef DEF_KWD
            #undef DEF_PUNCT
        };

        enum Punct_e {
            PUNCT_ERROR = 0,  // -"-

            #define DEF_KWD(NAME, STR)
            #define DEF_PUNCT(NAME, STR)  PUNCT_##NAME,

            #include "tokens.dsl.h"

            #undef DEF_KWD
            #undef DEF_PUNCT
        };

        FACTORIES(Token)

        bool ctor();

        // TODO: Constructors for different types

        void dtor();

        Type_e getType() const;

        /// Returns KWD_ERROR for non-kwd tokens
        Kwd_e getKwd() const;

        /// Returns PUNCT_ERROR for non-punct tokens
        Punct_e getPunct() const;

        // TODO: TOK_NAME interface

        // TODO: Implied type for TOK_NUM

        bool isInteger() const;

        unsigned long long asInt() const;

        double asDbl() const;

        bool isEnd() const;
        bool isErr() const;
        bool isName() const;
        bool isNum() const;
        bool isKwd() const;
        bool isPunct() const;

    private:
        Type_e type;

        union {
            // TOK_KWD
            Kwd_e kwd;

            // TOK_PUNCT
            Punct_e punct;

            // TOK_NAME
            struct {
                const char *start;
                unsigned length;
            };

            // TODO: Maybe a separate struct for numberInfo, and reference it by pointer? for size purposes?
            // TOK_NUM
            struct {
                unsigned long long integer;
                double fraction;
                int exp;  // TODO
                bool intFlag;
            };
        };

    };


    class Lexer {
    public:
        FACTORIES(Lexer)

        bool ctor();

        bool ctor(const FileBuf *src);

        void dtor();

        const Token *cur();

        const Token *peek(int delta);

        const Token *next();

        const Token *prev();

        unsigned backup();

        void restore(unsigned saved);

    private:
        const FileBuf *buf;

        Token *tokens;

        unsigned pos;
        unsigned size;
        unsigned capacity;

        static const unsigned DEFAULT_TOKENS_CAP = 32;

        bool resize(unsigned new_capacity);

        bool appendTok()

    };

}


#endif // LEXER_H

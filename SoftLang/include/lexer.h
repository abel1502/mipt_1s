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

        static const unsigned MAX_ID_LEN = 64;

        // WARNING: Should be manually adjusted
        static const unsigned MAX_PUNCT_LEN = 2;
        static const unsigned MAX_KWD_LEN = 5;

        FACTORIES(Token)

        bool ctor();

        bool ctorEnd();
        bool ctorErr();
        bool ctorName(const char *new_start, unsigned new_length);
        bool ctorNum(unsigned long long new_integer, double new_fraction, int new_exp, bool new_intFlag);
        bool ctorKwd(Kwd_e new_kwd);
        bool ctorPunct(Punct_e new_punct);

        void dtor();

        Type_e getType() const;

        /// Returns KWD_ERROR for non-kwd tokens
        Kwd_e getKwd() const;

        /// Returns PUNCT_ERROR for non-punct tokens
        Punct_e getPunct() const;

        const char *strKwd() const;

        const char *strPunct() const;

        const char *strKwdName() const;

        const char *strPunctName() const;

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

        void dump() const;

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
                int exp;
                bool intFlag;
            };
        };

    };


    class Lexer {
    public:
        static inline const char LINE_COMMENT_START[] = "//";
        static inline const char BLOCK_COMMENT_START[] = "/*";
        static inline const char BLOCK_COMMENT_END[] = "*/";

        static_assert(sizeof(LINE_COMMENT_START) == 3);

        FACTORIES(Lexer)

        bool ctor();

        bool ctor(const FileBuf *src);

        void dtor();

        bool parse();

        const Token *cur() const;

        const Token *peek(int delta) const;

        const Token *next();

        const Token *prev();

        unsigned backup() const;

        void restore(unsigned saved);

        void dump() const;

        const Token *getError() const;

        const Token *getEnd() const;

    private:
        const FileBuf *buf;

        Token *tokens;

        unsigned pos;
        unsigned size;
        unsigned capacity;

        static const unsigned DEFAULT_TOKENS_CAP = 32;

        bool resize(unsigned new_capacity);

        bool appendTok();

        static bool parseTok(Token *dest, FileBufIterator *iter);

        static bool parseNumber(Token *dest, FileBufIterator *iter);

        static bool parseIdentifier(Token *dest, FileBufIterator *iter);

        static bool parsePunct(Token *dest, FileBufIterator *iter);

        static void skipSpace(FileBufIterator *iter);

        static int recognizeDigit(char c);

        static Token::Kwd_e recognizeKwd(const char *src, unsigned *len);

        static Token::Punct_e recognizePunct(const char *src, unsigned *len);

        static bool isIdChar(char c);

    };

}


#endif // LEXER_H

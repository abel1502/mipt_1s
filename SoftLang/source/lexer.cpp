#include "lexer.h"

#include <cstdlib>
#include <cmath>
#include <cassert>
#include <cctype>
#include <cstring>


namespace SoftLang {

    bool Token::ctor() {
        type = TOK_ERROR;

        return false;
    }

    bool Token::ctorEnd() {
        type = TOK_END;

        return false;
    }

    bool Token::ctorErr() {
        type = TOK_ERROR;

        return false;
    }

    bool Token::ctorName(const char *new_start, unsigned new_length) {
        assert(new_start);

        type = TOK_NAME;
        start = new_start;
        length = new_length;

        return false;
    }

    bool Token::ctorNum(unsigned long long new_integer, double new_fraction, int new_exp, bool new_intFlag) {
        type = TOK_NUM;

        integer = new_integer;
        fraction = new_fraction;
        exp = new_exp;
        intFlag = new_intFlag;

        return false;
    }

    bool Token::ctorKwd(Token::Kwd_e new_kwd) {
        type = TOK_KWD;

        kwd = new_kwd;

        return false;
    }

    bool Token::ctorPunct(Token::Punct_e new_punct) {
        type = TOK_PUNCT;

        punct = new_punct;

        return false;
    }

    void Token::dtor() {
        type = TOK_ERROR;  // I'm too lazy to poison all the union fields
    }

    Token::Type_e Token::getType() const {
        return type;
    }

    Token::Kwd_e Token::getKwd() const {
        return isKwd() ? kwd : KWD_ERROR;
    }

    Token::Punct_e Token::getPunct() const {
        return isPunct() ? punct : PUNCT_ERROR;
    }

    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wswitch-enum"

    const char *Token::strKwd() const {
        assert(isKwd());

        switch (kwd) {
            #define DEF_KWD(NAME, STR) \
                case KWD_##NAME:  return STR;

            #define DEF_PUNCT(NAME, STR)

            #include "tokens.dsl.h"

            #undef DEF_KWD
            #undef DEF_PUNCT

        default:
            assert(false);
            return nullptr;
        }
    }

    const char *Token::strPunct() const {
        assert(isPunct());

        switch (punct) {
            #define DEF_KWD(NAME, STR)

            #define DEF_PUNCT(NAME, STR) \
                case PUNCT_##NAME:  return STR;

            #include "tokens.dsl.h"

            #undef DEF_KWD
            #undef DEF_PUNCT

        default:
            assert(false);
            return nullptr;
        }
    }

    const char *Token::strKwdName() const {
        assert(isKwd());

        switch (kwd) {
            #define DEF_KWD(NAME, STR) \
                case KWD_##NAME:  return #NAME;

            #define DEF_PUNCT(NAME, STR)

            #include "tokens.dsl.h"

            #undef DEF_KWD
            #undef DEF_PUNCT

        default:
            assert(false);
            return nullptr;
        }
    }

    const char *Token::strPunctName() const {
        assert(isPunct());

        switch (punct) {
            #define DEF_KWD(NAME, STR)

            #define DEF_PUNCT(NAME, STR) \
                case PUNCT_##NAME:  return #NAME;

            #include "tokens.dsl.h"

            #undef DEF_KWD
            #undef DEF_PUNCT

        default:
            assert(false);
            return nullptr;
        }
    }

    #pragma GCC diagnostic pop

    bool Token::isInteger() const {
        assert(isNum());

        return /*isNum() &&*/ intFlag;
    }

    unsigned long long Token::asInt() const {
        assert(isNum());

        if (isInteger() || (fraction == 0 && exp == 0))
            return integer;
        else
            return (unsigned long long)asDbl();  // To guarantee correct rounding
    }

    double Token::asDbl() const {
        assert(isNum());

        return (integer + fraction) * pow(10, exp);  // TODO: Maybe handle errors
    }

    bool Token::isEnd() const {
        return type == TOK_END;
    }

    bool Token::isErr() const {
        return type == TOK_ERROR;
    }

    bool Token::isName() const {
        return type == TOK_NAME;
    }

    bool Token::isNum() const {
        return type == TOK_NUM;
    }

    bool Token::isKwd() const {
        return type == TOK_KWD;
    }

    bool Token::isPunct() const {
        return type == TOK_PUNCT;
    }

    void Token::dump() const {
        switch (getType()) {
        case Token::TOK_END:
            printf(".");
            break;

        case Token::TOK_ERROR:
            printf("E");
            break;

        case Token::TOK_KWD:
            printf("K<%s>", strKwdName());
            break;

        case Token::TOK_PUNCT:
            printf("P<%s>", strPunctName());
            break;

        case Token::TOK_NAME:
            printf("N<%.*s>", length, start);
            break;

        case Token::TOK_NUM:
            if (isInteger()) {
                printf("#<%llu>", asInt());
            } else {
                printf("#<%lg>", asDbl());
            }

            break;
        default:
            assert(false);
            break;
        }
    }

    //--------------------------------------------------------------------------------

    bool Lexer::ctor() {
        buf = nullptr;

        tokens = (Token *)calloc(DEFAULT_TOKENS_CAP, sizeof(Token));
        if (!tokens)
            return true;

        pos = 0;
        size = 0;
        capacity = DEFAULT_TOKENS_CAP;

        return false;
    }

    bool Lexer::ctor(const FileBuf *src) {
        TRY_B(ctor());

        buf = src;

        return false;
    }

    void Lexer::dtor() {
        // free(buf); // We DON'T own it, so we don't destroy it either
        buf = nullptr;

        for (unsigned i = 0; i < size; ++i) {
            // With tokens == nullptr, capacity will always be 0, so this loop won't trigger anyway
            tokens[i].dtor();
        }

        free(tokens);
        tokens = nullptr;

        pos = 0;
        size = 0;
        capacity = 0;
    }

    bool Lexer::parse() {
        FileBufIterator iter{};
        TRY_B(iter.ctor(buf));

        while (!getEnd()) {
            TRY_BC(appendTok(), iter.dtor());

            assert(size > 0);

            TRY_BC(parseTok(&tokens[size - 1], &iter), iter.dtor());

            if (getError()) {
                ERR("Syntax error at pos #%zu (near \"%.5s\")", iter.getPos(), iter.getPtr());

                iter.dtor();

                return false;  // Still not considered an error, but is marked via the TOK_ERROR token
            }
        }

        iter.dtor();

        return false;
    }

    const Token *Lexer::cur() const {
        assert(!getError() && getEnd());

        if (pos >= size)
            return getEnd();

        return &tokens[pos];
    }

    const Token *Lexer::peek(int delta) const {
        assert(!getError() && getEnd());

        if (pos + delta >= size)
            return getEnd();

        return &tokens[pos + delta];
    }

    const Token *Lexer::next() {
        const Token *tmp = cur();

        pos++;

        return tmp;
    }

    const Token *Lexer::prev() {
        const Token *tmp = cur();

        pos--;

        return tmp;
    }

    unsigned Lexer::backup() const {
        return pos;
    }

    void Lexer::restore(unsigned saved) {
        pos = saved;
    }

    const Token *Lexer::getError() const {
        if (size != 0 && tokens[size - 1].isErr())
            return &tokens[size - 1];

        return nullptr;
    }

    const Token *Lexer::getEnd() const {
        if (size != 0 && tokens[size - 1].isEnd())
            return &tokens[size - 1];

        return nullptr;
    }

    bool Lexer::resize(unsigned new_capacity) {
        assert(new_capacity >= capacity);

        Token *newTokens = (Token *)realloc(tokens, new_capacity * sizeof(Token));
        TRY_B(!newTokens);

        tokens = newTokens;
        capacity = new_capacity;

        return false;
    }

    bool Lexer::appendTok() {
        assert(capacity != 0);

        if (size + 1 >= capacity) {
            TRY_B(resize(capacity * 2));
        }

        size++;

        assert(size <= capacity);

        return false;
    }

    bool Lexer::parseTok(Token *dest, FileBufIterator *iter) {
        assert(dest);
        assert(iter);

        iter->skipSpace();

        char tmp = iter->cur();

        if (tmp == '\0') {
            TRY_B(dest->ctorEnd());
            return false;
        }

        if (isdigit(tmp) || (tmp == '.' && isdigit(iter->peek(1)))) {
            return parseNumber(dest, iter);
        }

        if (isalpha(tmp)) {
            return parseIdentifier(dest, iter);
        }

        return parsePunct(dest, iter);
    }

    bool Lexer::parseNumber(Token *dest, FileBufIterator *iter) {
        int base = 10;

        // Note: We don't parse sign here, if present it will be treated as PUNCT_SUB

        if (iter->cur() == '0') {
            switch (iter->peek(1)) {
            case 'B':
            case 'b':
                base = 2;

                iter->next();
                iter->next();

                break;

            case 'O':
            case 'o':
                base = 8;

                iter->next();
                iter->next();

                break;

            case 'X':
            case 'x':
                base = 16;

                iter->next();
                iter->next();

                break;

            default:
                break;
            }
        }

        unsigned long long integer = 0;
        int integerLen = 0;

        int curDigit = recognizeDigit(iter->cur());

        while (curDigit >= 0 && curDigit < base) {
            iter->next();

            integerLen++;  // TODO: limit?

            integer = integer * base + curDigit;

            curDigit = recognizeDigit(iter->cur());
        }

        bool isInteger = true;
        double fraction = 0;
        int exp = 0;

        if (iter->cur() == '.' && (integerLen > 0 || recognizeDigit(iter->peek(1)))) {
            iter->next();

            isInteger = false;

            double fractionCoef = 1;

            curDigit = recognizeDigit(iter->cur());

            while (curDigit >= 0 && curDigit < base) {
                iter->next();

                fractionCoef /= base;

                fraction += fractionCoef * curDigit;

                curDigit = recognizeDigit(iter->cur());
            }
        }

        if (base == 10 && (integerLen > 0 || !isInteger) && (iter->cur() == 'e' || iter->cur() == 'E')) {
            iter->next();

            isInteger = false;

            int expLen = 0;
            int expSign = 1;

            switch (iter->cur()) {
            case '+':
                iter->next();
                break;
            case '-':
                expSign = -1;
                iter->next();
                break;
            default:
                break;
            }

            curDigit = recognizeDigit(iter->cur());

            while (curDigit >= 0 && curDigit < base) {
                iter->next();

                expLen++;

                exp = exp * base + curDigit;

                curDigit = recognizeDigit(iter->cur());
            }

            exp *= expSign;
        }

        if (integerLen == 0 && isInteger) {
            ERR("Syntex error: malformed number");

            TRY_B(dest->ctorErr());

            return false;
        }

        TRY_B(dest->ctorNum(integer, fraction, exp, isInteger));

        return false;
    }

    bool Lexer::parseIdentifier(Token *dest, FileBufIterator *iter) {
        const char *idStart = iter->getPtr();
        unsigned len = 0;

        while (len < Token::MAX_ID_LEN && isIdChar(iter->cur())) {
            ++len;
            iter->next();
        }

        if (len == Token::MAX_ID_LEN && isIdChar(iter->cur())) {
            ERR("Syntax error: identifier too long");  // TODO: wrapper

            TRY_B(dest->ctorErr());

            return false;
        }

        unsigned kwdLen = 0;

        Token::Kwd_e kwd = recognizeKwd(idStart, &kwdLen);

        if (kwd != Token::KWD_ERROR && kwdLen == len) {
            TRY_B(dest->ctorKwd(kwd));

            return false;
        }

        TRY_B(dest->ctorName(idStart, len));

        return false;
    }

    bool Lexer::parsePunct(Token *dest, FileBufIterator *iter) {
        unsigned len = 0;

        Token::Punct_e punct = recognizePunct(iter->getPtr(), &len);

        if (punct == Token::PUNCT_ERROR) {
            ERR("Syntax error: unrecognized punctuation symbol");

            TRY_B(dest->ctorErr());

            return false;
        }

        for (unsigned i = 0; i < len; ++i)
            iter->next();

        TRY_B(dest->ctorPunct(punct));

        return false;
    }

    int Lexer::recognizeDigit(char c) {
        if (isdigit(c))
            return c - '0';

        if ('A' <= c && c <= 'F')
            return c - 'A' + 10;

        if ('a' <= c && c <= 'f')
            return c - 'a' + 10;

        return -1;
    }

    Token::Kwd_e Lexer::recognizeKwd(const char *src, unsigned *len) {
        #define DEF_KWD(NAME, STR)                          \
            if (sizeof(STR) - 1 > maxLen &&                 \
                strncmp(src, STR, sizeof(STR) - 1) == 0) {  \
                maxLen = sizeof(STR) - 1;                   \
                bestMatch = Token::KWD_##NAME;              \
            }

        #define DEF_PUNCT(NAME, STR)

        unsigned maxLen = 0;
        Token::Kwd_e bestMatch = Token::KWD_ERROR;

        #include "tokens.dsl.h"

        *len = maxLen;
        return bestMatch;

        #undef DEF_KWD
        #undef DEF_PUNCT
    }

    Token::Punct_e Lexer::recognizePunct(const char *src, unsigned *len) {
        #define DEF_KWD(NAME, STR)

        #define DEF_PUNCT(NAME, STR)                        \
            if (sizeof(STR) - 1 > maxLen &&                 \
                strncmp(src, STR, sizeof(STR) - 1) == 0) {  \
                maxLen = sizeof(STR) - 1;                   \
                bestMatch = Token::PUNCT_##NAME;            \
            }

        unsigned maxLen = 0;
        Token::Punct_e bestMatch = Token::PUNCT_ERROR;

        #include "tokens.dsl.h"

        *len = maxLen;
        return bestMatch;

        #undef DEF_KWD
        #undef DEF_PUNCT
    }

    bool Lexer::isIdChar(char c) {
        return isalnum(c) || c == '_';
    }

    void Lexer::dump() const {
        for (unsigned i = 0; i < size; ++i) {
            tokens[i].dump();

            printf(" ");
        }

        printf("\n");
    }

}

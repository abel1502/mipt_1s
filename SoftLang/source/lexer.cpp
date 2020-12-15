#include "lexer.h"

#include <cstdlib>
#include <cmath>
#include <cassert>


namespace SoftLang {

    bool Token::ctor() {
        type = TOK_ERROR;

        length = 0;
        //value = nullptr;

        return false;
    }

    void Token::dtor() {
        type = TOK_ERROR;

        length = 0;
        //value = nullptr;  // We don't own it, so no free here
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

    //--------------------------------------------------------------------------------

    bool Lexer::ctor() {
        buf = nullptr;

        tokens = (Token *)calloc(DEFAULT_TOKENS_CAP, sizeof(Token));
        if (!tokens)
            return false;

        pos = 0;
        size = 0;
        capacity = DEFAULT_TOKENS_CAP;

        return true;
    }

    bool Lexer::ctor(const FileBuf *src) {
        TRY(ctor());

        buf = src;

        FileBufIterator iter{};
        TRY(iter.ctor(buf));

        bool keep

        while (!iter.isEof()) {
            //
            if (tokens[size - 1].isErr()) {
                ERR("Syntax error at pos #%zu (near \"%5s\")", iter->getPos(), iter->getCtx());

                return true;
            }
        }
        TRY(appendTok());
        TRY(tokens[size - 1].ctorEnd());

        iter.dtor();

        return false;
    }

    void Lexer::dtor() {
        // free(buf); // We DON'T own it, so we don't destroy it either
        buf = nullptr;

        for (unsigned i = 0; i < capacity; ++i) {
            // With tokens == nullptr, capacity will always be 0, so this loop won't trigger anyway
            tokens[i].dtor();
        }

        free(tokens);
        tokens = nullptr;

        pos = 0;
        size = 0;
        capacity = 0;
    }

}

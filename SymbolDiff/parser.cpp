#include "parser.h"

#include <cstdlib>
#include <cassert>
#include <cstring>
#include <cctype>
#include <sys/stat.h>


namespace SymbolDiff {

    Parser *Parser::ctor() {
        size = 0;
        buf = nullptr;

        pos = 0;
        errPos = 0;

        fSpace = false;
        fLine = false;

        return this;
    }

    void Parser::dtor() {
        free(buf);
    }

    unsigned Parser::fsize(FILE *ifile) {
        struct stat fbuf = {};

        REQUIRE(fstat(fileno(ifile), &fbuf) == 0);

        return fbuf.st_size + 1;
    }

    void Parser::feed(FILE *ifile) {
        assert(ifile);
        assert(!buf);

        size = fsize(ifile);

        buf = (char *)calloc(size, 1);
        REQUIRE(buf);

        REQUIRE(fread(buf, 1, size - 1, ifile) == size - 1);
    }

    void Parser::feed(const char *src) {
        assert(src);
        assert(!buf);

        size = strlen(src) + 1;

        buf = (char *)calloc(size, 1);
        REQUIRE(buf);

        strncpy(buf, src, size - 1);
    }

    bool Parser::isEof() {
        assert(isValid());

        return pos >= size - 1;
    }

    bool Parser::wasSpace() {
        assert(isValid());

        return fSpace;
    }

    bool Parser::wasLine() {
        assert(isValid());

        return fLine;
    }

    char Parser::cur() {
        assert(isValid());

        if (isEof())
            return '\0';

        return buf[pos];
    }

    char Parser::next() {
        assert(isValid());

        fSpace = false;
        fLine = false;

        char res = buf[pos];

        do {
            fSpace |= isspace(buf[pos]);
            fLine  |= buf[pos] == '\n';

            pos++;
        } while (!isEof() && isspace(buf[pos]));

        return res;
    }

    char Parser::prev() {
        assert(isValid());

        // TODO: Properly restore correct fSpace and fLine
        fSpace = false;
        fLine = false;

        char res = buf[pos];

        if (pos == 0)
            return '\0';

        do {
            fSpace |= isspace(buf[pos]);
            fLine  |= buf[pos] == '\n';

            pos--;
        } while (pos > 0 && isspace(buf[pos]));

        return res;
    }

    void Parser::logErr() {
        ERR("Parsing failure at #%u/%u - \"%.5s\" (0x%02x)\n", errPos, size - 1, buf + errPos, buf[errPos]);
    }


    // ===== [ Parser DSL ] =====

    #define PSAVE()        unsigned savePos_ = pos;

    #define PRESTORE()     pos = savePos_;

    #if DBG_PARSER
    #define PGOOD()        printf("Yes %s.\n", __func__); return false;

    #define PBAD()         printf("No %s.\n", __func__); errPos = pos; PRESTORE(); return true;
    #else
    #define PGOOD()        return false;

    #define PBAD()         errPos = pos; PRESTORE(); return true;
    #endif

    #define PEXPECT(C)     if (next() != C) { PBAD(); }

    // ===========================

    bool Parser::parse(ExprNode *dest) {
        if (parseExprNode(dest)) {
            logErr();

            return true;
        }

        if (!isEof()) {
            errPos = pos;

            ERR("Garbage at the end of stream.");
            logErr();

            return true;
        }

        return false;
    }

    bool Parser::parseInt(long long *dest) {
        PSAVE();

        #if DBG_PARSER
        printf(">> Num %.8s\n", buf + pos);
        #endif

        long long result = 0;

        bool invert = false;

        // Won't actually trigger, but I decided to still implement it
        if (cur() == '-') {
            next();

            invert = true;
        }

        if (isdigit(cur())) {
            result = next() - '0';
        } else {
            PBAD();
        }

        while (isdigit(cur()) && !wasSpace()) {
            result *= 10;
            result += next() - '0';
        }

        *dest = invert ? -result : result;

        PGOOD();
    }

    bool Parser::parseBinOp(int *dest) {
        PSAVE();

        #if DBG_PARSER
        printf(">> BinOp %.8s\n", buf + pos);
        #endif

        #define CASE_TPL(NAME, STR)                                 \
            if (strncmp(buf + pos, STR, sizeof(STR) - 1) == 0) {    \
                pos += sizeof(STR) - 2;                             \
                next();                                             \
                *dest = BinOp_##NAME;                               \
            } else

        #include "tpl_BinOp.h"

        {
            PBAD();
        }

        PGOOD();

        #undef CASE_TPL
    }

    bool Parser::parseUnOp(int *dest) {
        PSAVE();

        #if DBG_PARSER
        printf(">> UnOp %.8s\n", buf + pos);
        #endif

        #define CASE_TPL(NAME, STR)                                 \
            if (strncmp(buf + pos, STR, sizeof(STR) - 1) == 0) {    \
                pos += sizeof(STR) - 2;                             \
                next();                                             \
                *dest = UnOp_##NAME;                                \
            } else

        #include "tpl_UnOp.h"

        {
            PBAD();
        }

        PGOOD();

        #undef CASE_TPL
    }

    bool Parser::parseBinOpNode(ExprNode *dest) {
        PSAVE();

        #if DBG_PARSER
        printf(">> BinNode %.8s\n", buf + pos);
        #endif

        ExprNode *left  = ExprNode::create();
        ExprNode *right = ExprNode::create();
        REQUIRE(left && right);  // If any of these aren't created, we can't resume the parsing

        BinOp_e binOp = (BinOp_e)0;

        if (parseExprNode(left) || parseBinOp((int *)&binOp) || parseExprNode(right)) {
            left->destroy();
            right->destroy();

            PBAD();
        }

        dest->ctorBinOp(binOp, left, right);

        PGOOD();
    }

    bool Parser::parseUnOpNode(ExprNode *dest) {
        PSAVE();

        #if DBG_PARSER
        printf(">> UnNode %.8s\n", buf + pos);
        #endif

        ExprNode *child  = ExprNode::create();
        UnOp_e unOp = (UnOp_e)0;

        if (parseUnOp((int *)&unOp) || parseExprNode(child)) {
            child->destroy();

            PBAD();
        }

        dest->ctorUnOp(unOp, child);

        PGOOD();
    }

    bool Parser::parseExprNode(ExprNode *dest) {
        PSAVE();

        #if DBG_PARSER
        printf(">> Expr %.8s\n", buf + pos);
        #endif

        PEXPECT('(');

        if (!parseBinOpNode(dest)) {
            PEXPECT(')');
            PGOOD();
        } else if (isdigit(cur())) {
            long long value = 0;

            if (parseInt(&value)) {
                PBAD();
            }

            dest->ctorConst(value);

            PEXPECT(')');
            PGOOD();
        } else if (!parseUnOpNode(dest)) {
            PEXPECT(')');
            PGOOD();
        } else if (islower(cur())) {
            char varName = next();

            dest->ctorVar(varName);

            PEXPECT(')');
            PGOOD();
        }

        PBAD();
    }

    #undef PSAVE
    #undef PRESTORE
    #undef PGOOD
    #undef PBAD
    #undef PEXPECT


    bool Parser::isValid() {
        return buf;
    }
}

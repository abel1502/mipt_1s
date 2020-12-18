#ifndef PARSER_H
#define PARSER_H

#include "general.h"
#include "filebuf.h"
#include "lexer.h"
#include "ast.h"


namespace SoftLang {

    class TypeSpec;
    class Var;
    class Scope;
    class BinaryExpr;
    class UnaryExpr;
    class Expression;
    class Statement;
    class Code;
    class Function;
    class Program;

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

        inline const Token *cur() const;

        inline const Token *peek(int delta) const;

        inline const Token *next();

        inline const Token *prev();

        inline unsigned backup() const;

        inline void restore(unsigned saved);

        Error_e parse(Program *prog);

    private:

        Lexer lexer;

        Error_e parse_FUNC_DEFS(Program *prog);

        Error_e parse_FUNC_DEF(Program *prog);

        Error_e parse_FUNC_ARGS_DEF(Function *func);

        Error_e parse_FUNC_ARG_DEF(Var *arg);

        Error_e parse_TYPESPEC(TypeSpec *ts);

        Error_e parse_STMTS(Code *code);

        Error_e parse_STMT(Statement *stmt);

        Error_e parse_COMPOUND_STMT(Code *code);

        Error_e parse_COMPOUND_STMT(Statement *stmt);

        Error_e parse_RETURN_STMT(Statement *stmt);

        Error_e parse_LOOP_STMT(Statement *stmt);

        Error_e parse_COND_STMT(Statement *stmt);

        Error_e parse_VARDECL_STMT(Statement *stmt);

        Error_e parse_EXPR_STMT(Statement *stmt);

        Error_e parse_EXPR(Expression *expr);

        Error_e parse_ASGN_EXPR(Expression *expr);

        /*Error_e parse_OR_EXPR(Expression *expr);

        Error_e parse_AND_EXPR(Expression *expr);*/

        Error_e parse_CMP_EXPR(Expression *expr);

        Error_e parse_ADD_EXPR(Expression *expr);

        Error_e parse_MUL_EXPR(Expression *expr);

        Error_e parse_UNARY_EXPR(Expression *expr);

        Error_e parse_VAR_EXPR(Expression *expr);

        Error_e parse_VAR(const Token **name);

        Error_e parse_VARDECL(Var *var);

        Error_e parse_FUNC(const Token **name);

        Error_e parse_FUNC_CALL(Expression *expr);

        Error_e parse_FUNC_ARGS(Expression *expr);

    };

}


#endif // PARSER_H

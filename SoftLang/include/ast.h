#ifndef AST_H
#define AST_H

#include "vtable.h"


namespace SoftLang {

    class Program {
    };

    class Function {
    };

    class Code {
    };

    class TypeSpec {
    };

    class Var {
    };

    class Scope {
    };

    class Statement {  // Abstract
    };

    class Expression {  // Abstract, high-level
        /*
        union {
            BinaryExpr bin;
            UnaryExpr un;
        };
        */
    };

    class BinaryExpr {  // Abstract
    };

}


#endif // AST_H

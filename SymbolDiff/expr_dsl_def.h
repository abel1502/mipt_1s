#define ADD_(A, B)      CCRN(ExprNode, ctorBinOp, BinOp_Add, A, B)
#define SUB_(A, B)      CCRN(ExprNode, ctorBinOp, BinOp_Sub, A, B)
#define MUL_(A, B)      CCRN(ExprNode, ctorBinOp, BinOp_Mul, A, B)
#define DIV_(A, B)      CCRN(ExprNode, ctorBinOp, BinOp_Div, A, B)
#define POW_(A, B)      CCRN(ExprNode, ctorBinOp, BinOp_Pow, A, B)

#define NEG_(A)         CCRN(ExprNode, ctorUnOp, UnOp_Neg, A)
#define SIN_(A)         CCRN(ExprNode, ctorUnOp, UnOp_Sin, A)
#define COS_(A)         CCRN(ExprNode, ctorUnOp, UnOp_Cos, A)
#define LN_(A)          CCRN(ExprNode, ctorUnOp, UnOp_Ln, A)

#define COPY_(A)        VCALL(A, copy)
#define DIFF_(A)        VCALL(A, diff, by, logFile)
#define DIFFBY_(A, B)   VCALL(A, diff, B, logFile)
#define CONST_(A)       CCRN(ExprNode, ctorConst, A)
#define VAR_(A)         CCRN(ExprNode, ctorVar, A)

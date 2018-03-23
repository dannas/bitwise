typedef struct Expr Expr;

typedef enum ExprKind {
    EXPR_NONE,
    EXPR_INT,
    EXPR_UNARY,
    EXPR_BINARY
} ExprKind;

typedef struct UnaryExpr {
    TokenKind op;
    Expr *expr;
} UnaryExpr;

typedef struct BinaryExpr {
    TokenKind op;
    Expr *lhs;
    Expr *rhs;
} BinaryExpr;

struct Expr {
    ExprKind kind;
    union {
        uint64_t int_val;
        UnaryExpr unary;
        BinaryExpr binary;
    };
};

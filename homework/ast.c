Expr* expr_new(ExprKind kind) {
    Expr *e = malloc(sizeof(Expr));
    e->kind = kind;
    return e;
}

Expr *expr_int(uint64_t val) {
    Expr *e = expr_new(EXPR_INT);
    e->int_val = val;
    return e;
}

Expr* expr_unary(TokenKind op, Expr *expr) {
    Expr *e = expr_new(EXPR_UNARY);
    e->unary.op = op;
    e->unary.expr = expr;
    return e;
}

Expr* expr_binary(TokenKind op, Expr *lhs, Expr *rhs) {
    Expr *e = expr_new(EXPR_BINARY);
    e->binary.op = op;
    e->binary.lhs = lhs;
    e->binary.rhs = rhs;
    return e;
}

void print(Expr *e) {
    switch (e->kind) {
    case EXPR_INT:
        printf("%llu", e->int_val);
        break;
    case EXPR_UNARY:
        printf("(%c", e->unary.op);
        print(e->unary.expr);
        printf(")");
        break;
    case EXPR_BINARY:
        printf("(%c", e->binary.op);
        print(e->binary.lhs);
        print(e->binary.rhs);
        printf(")");
        break;
    default:
        printf("kind not handled\n");
    }
}

void print_ast_test() {
    printf("print-tests================\n");
    print(expr_int(42));
    print(expr_unary('-', expr_int(80)));
}

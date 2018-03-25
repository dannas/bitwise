typedef enum TokenKind {
// Reserve first 128 values for one-char tokens
    TOKEN_LAST_CHAR = 127,
    TOKEN_INT,
    TOKEN_NAME,
    // ...
} TokenKind;

// Warning: This returns a pointer to a static internal buffer, so it'll be overwritten next call.
const char *token_kind_name(TokenKind kind) {
    static char buf[256];
    switch (kind) {
    case TOKEN_INT:
        sprintf(buf, "integer");
        break;
    case TOKEN_NAME:
        sprintf(buf, "name");
        break;
    default:
        if (kind < 128 && isprint(kind)) {
            sprintf(buf, "%c", kind);
        } else {
            sprintf(buf, "<ASCII %d>", kind);
        }
        break;
    }
    return buf;
}

typedef struct Token {
    TokenKind kind;
    const char *start;
    const char *end;
    union {
        int val;
        const char *name;
    };
} Token;

Token token;
const char *stream;

const char *keyword_if;
const char *keyword_for;
const char *keyword_while;

void init_keywords() {
    keyword_if = str_intern("if");
    keyword_for = str_intern("for");
    keyword_while = str_intern("while");
    // ...
}

void next_token() {
    token.start = stream;
    switch (*stream) {
    case '0'...'9': {
        int val = 0;
        while (isdigit(*stream)) {
            val *= 10;
            val += *stream++ - '0';
        }
        token.kind = TOKEN_INT;
        token.val = val;
        break;
    }
    case 'a'...'z':
    case 'A'...'Z':
    case '_':
        while (isalnum(*stream) || *stream == '_') {
            stream++;
        }
        token.kind = TOKEN_NAME;
        token.name = str_intern_range(token.start, stream);
        break;
    default:
        token.kind = *stream++;
        break;
    }
    token.end = stream;
}

void init_stream(const char *str) {
    stream = str;
    next_token();
}

void print_token(Token token) {
    switch (token.kind) {
    case TOKEN_INT:
        printf("TOKEN INT: %d\n", token.val);
        break;
    case TOKEN_NAME:
        printf("TOKEN NAME: %.*s\n", (int)(token.end - token.start), token.start);
        break;
    default:
        printf("TOKEN '%c'\n", token.kind);
        break;
    }
}

static inline bool is_token(TokenKind kind) {
    return token.kind == kind;
}

static inline bool is_token_name(const char *name) {
    return token.kind == TOKEN_NAME && token.name == name;
}

static inline bool match_token(TokenKind kind) {
    if (is_token(kind)) {
        next_token();
        return true;
    } else {
        return false;
    }
}

static inline bool expect_token(TokenKind kind) {
    if (is_token(kind)) {
        next_token();
        return true;
    } else {
        fatal("expected token %s, got %s", token_kind_name(kind), token_kind_name(token.kind));
        return false;
    }
}

void lex_test() {
    char *source = "XY+(XY)_HELLO1,234+FOO!994";
    stream = source;
    next_token();
    while (token.kind) {
        next_token();
    }
}

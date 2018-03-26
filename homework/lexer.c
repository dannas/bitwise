typedef enum TokenKind {
// Reserve first 128 values for one-char tokens
    TOKEN_LAST_CHAR = 127,
    TOKEN_INT,
    TOKEN_NAME,
    TOKEN_LSHIFT,
    TOKEN_RSHIFT
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
    case TOKEN_LSHIFT:
        sprintf(buf, "<<");
        break;
    case TOKEN_RSHIFT:
        sprintf(buf, ">>");
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
    case '<':
        token.kind = *stream++;
        if (*stream == '<')  {
            token.kind = TOKEN_LSHIFT;
            stream++;
        }
        break;
    case '>':
        token.kind = *stream++;
        if (*stream == '>') {
            token.kind = TOKEN_RSHIFT;
            stream++;
        }
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

#define assert_token(x) (assert(match_token(x)))
#define assert_token_eof() (assert(match_token('\0')))
#define assert_token_int(x) assert(token.val == (x) && match_token(TOKEN_INT))

void lex_test() {
    init_stream("");
    assert_token_eof();

    init_stream("1");
    assert_token_int(1);

    init_stream("1+2");
    assert_token_int(1);
    assert_token('+');
    assert_token_int(2);

    init_stream("1<2");
    assert_token_int(1);
    assert_token('<');
    assert_token_int(2);

    init_stream("1<<2");
    assert_token_int(1);
    assert_token(TOKEN_LSHIFT);
    assert_token_int(2);

    init_stream("1>2");
    assert_token_int(1);
    assert_token('>');
    assert_token_int(2);

    init_stream("1>>2");
    assert_token_int(1);
    assert_token(TOKEN_RSHIFT);
    assert_token_int(2);
}

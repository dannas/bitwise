#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <assert.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdarg.h>

#include "common.c"
#include "lexer.c"
#include "ast.h"
#include "ast.c"

/*
  Problems I encountered:
  Didn't parenthesize ((b) = arr__grow((b), (n), sizeof(*b))) which caused an
  error about non-existing lvalue for arr_push(). So lesson learned: the
  ternary operator is right associative and an assignment expression returns
  the type.

  I returned new_hdr instead of new_hdr->buf from arr__grow.
  I didn't assign values to the ptr->len and ptr->cap at the right place

  QtCreators watch expression pane choked on (ArrHdr*)((uintptr_t)vals - offsetof(ArrHdr, buf))
  Turns out you can't use macros for gdb expressions.

  QtCreator choked on *(ArrHdr*)((uintptr_t)vals - 16)
  Turns out you can't dereference a cast (not sure if that is the correct
  conclusion, but the expression was not valid)

  This watch expression worked (ArrHdr*)((uintptr_t)vals - 16). But it printed
  the struct members in opposite declaration order.

  In arr__grow, I initialized new_hdr->len to len instead of the correct 0. I
  should have. The len is supposed to be incremented in arr_push.

  I called realloc with ptr as argument instead of arr__hdr(ptr).
  Lucky for me, realloc asserted immediately.
*/

// Stretchy buffer. My reimplementation from scratch.
// Tried using statements instead but it didn't make the code clearer.
typedef struct ArrHdr {
    size_t len;
    size_t cap;
    char buf[];
} ArrHdr;

#define arr__hdr(b) ((ArrHdr*)((uintptr_t)b - offsetof(ArrHdr, buf)))
#define arr__fits(b, n) (arr_len(b) + (n) < arr_cap(b))
#define arr__fit(b, n) (arr__fits(b, n) ? 0 : ((b) = arr__grow((b), (n), sizeof(*b))))

#define arr_len(b) ((!b) ? 0 : arr__hdr((b))->len)
#define arr_cap(b) ((!b) ? 0 : arr__hdr((b))->cap)
#define arr_push(b, val) (arr__fit(b, 1), (b)[arr__hdr(b)->len++] = (val))
#define arr_free(b) ((!b) ? 0 : free(arr__hdr(b)), (b) = NULL)

void* arr__grow(void *ptr, size_t len, size_t num_bytes) {
    size_t new_cap = arr_cap(ptr) * 2 + 1;
    ArrHdr *new_hdr = NULL;
    if (ptr) {
        new_hdr = xrealloc(arr__hdr(ptr), len * num_bytes);
    } else {
        new_hdr = xmalloc(len * num_bytes);
        new_hdr->len = 0;
    }
    new_hdr->cap = new_cap;
    return new_hdr->buf;
}

void arr_test() {
    // Start of empty.
    int *vals = NULL;
    assert(arr_len(vals) == 0);
    assert(arr_cap(vals) == 0);

    // Add first elt. Triggers malloc.
    arr_push(vals, 1);
    assert(arr_len(vals) == 1);
    assert(arr_cap(vals) == 1);

    // Add second elt. Triggers realloc.
    arr_push(vals, 2);
    assert(arr_len(vals) == 2);
    assert(arr_cap(vals) == 3);

    // Freeing should reset the pointer.
    arr_free(vals);
    assert(vals == NULL);
}

/*
 Here's the operators we're supposed to be able to parse:

        unary -, unary ~    (right associative)
        * / % << >> &       (left associative)
        + - | ^             (left associative)

 First attempt

        expr :    expr '+' expr
                  expr '-' expr
                  expr '*' expr
                  ....

 This grammar is left-recursive. The parser will not terminate. Example:

        void expr() { expr(); match('+'); }

 And the grammar doesn't handle presedence either.

 Let's try something similar to what I think I've seen in K&R:

        program : expr*
        expr :    term { '+' term }
        term :    factor { '*' factor }
        factor :  number
                  '(' expr ')'

 So this sort of works, but I have a hard time getting my head around why it works. I guess I need to write
 more parse trees on paper.

 I could print the expressions in postfix form by simply printing the after the terms/factors/powers.
 But prefix form AFAICT, requires that I return each expression in some form, and then combine them.

 I've used a simplified version of the shunting yard algorithm before that didn't take presedence into account.

 From here I got stuck and did an Internet search and found http://www.engr.mun.ca/~theo/Misc/exp_parsing.htm

 Can I write an S-expression form without having to return a value further up the call stack?
 I guess, using a linked tree would have been a better choice than my string allocation and concatenation.
 TODO: Another explanation of the shunting yard algorithm:
 https://eli.thegreenplace.net/2009/03/20/a-recursive-descent-parser-with-an-infix-expression-evaluator

 How do I write a grammar for the the operators in the table?
 I need three levels of presedence. I think I have the correct expr and term production rule, but how fix
 factor? A factor can be an expr. This confuses me. I thought that production rules should all reduce the possible
 number of choices.

 expr   = expr {'+' term}
 term   = term {'*' factor}
 factor = v |  '(' expr ')' | '-' term
*/

char* print_expr();
char* print_term();
char* print_factor();

// TODO(dannas): Figure out how to manage deallocation of all small strings
// TODO(dannas): Figure out how to merge all these allocations.
char* make_string(char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    int size = vsnprintf(NULL, 0, fmt, args);

    // Add space for '\0'
    size++;

    char *buf = xmalloc(size);

    va_start(args, fmt);
    vsnprintf(buf, size, fmt, args);
    va_end(args);

    return buf;
}

char* print_expr() {
    char *t = print_term();
    while (is_token('+') || is_token('-') || is_token('|') || is_token('^')) {
        char op = token.kind;
        next_token();
        char *t1 = print_term();
        t = make_string("(%c %s %s)", op, t, t1);
    }
    return t;
}

char* print_term() {
    char* t = print_factor();
    // TODO(dannas): Add two-byte
    while (is_token('*') || is_token('/') || is_token('%')) {
        char op = token.kind;
        next_token();
        char* t1 = print_factor();
        t = make_string("(%c %s %s)", op, t, t1);
    }
    return t;
}

char* print_factor() {
    char *t = NULL;
    if (is_token(TOKEN_INT)) {
        t = make_string("%d", token.val);
        next_token();
    } else if (is_token('(')) {
        next_token();
        t = print_expr();
        expect_token(')');
    } else if (is_token('-') || is_token('~')) {
        char c = token.kind;
        next_token();
        t = make_string("(%c %s)", c, print_term());
    }

    return t;
}

#define TEST_PRINT(expr) init_stream(expr); printf("%s\n", print_expr())
void print_test() {

    TEST_PRINT("1*2");
    TEST_PRINT("1+2+3");
    TEST_PRINT("1*2+3");
    TEST_PRINT("1+2*3");
    TEST_PRINT("1*2*3");
    TEST_PRINT("1*1*1+1");
    TEST_PRINT("2*3-4*5*6+7");
    TEST_PRINT("2*3-4/5*6+7");
    TEST_PRINT("1*-2*(1+3+4)");
}


int main() {
    buf_test();
    lex_test();
    str_intern_test();
    print_test();

    // TODO(dannas): Replace later
    arr_test();
    return 0;
}


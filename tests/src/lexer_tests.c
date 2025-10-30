// Implementace prekladace imperativniho jazyka IFJ25
// lexer_tests.c by Jan Frantisek "xlogin00" Levicek on 10/22/25.
//

#include "/src/scanner.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct {
    const char *name;
    token_type_t value;
} name_val_t;

#define ENTRY(x) { #x, x }

static const name_val_t MAP[] = {
        ENTRY(TT_EOF),
        ENTRY(TT_EOL),
        ENTRY(TT_IDENTIFIER),
        ENTRY(TT_NULL),
        ENTRY(TT_INT),
        ENTRY(TT_FLOAT),
        ENTRY(TT_STRING),

        ENTRY(TT_KEYWORD_CLASS),
        ENTRY(TT_KEYWORD_IF),
        ENTRY(TT_KEYWORD_ELSE),
        ENTRY(TT_KEYWORD_IS),
        ENTRY(TT_KEYWORD_RETURN),
        ENTRY(TT_KEYWORD_VAR),
        ENTRY(TT_KEYWORD_WHILE),
        ENTRY(TT_KEYWORD_IFJ),
        ENTRY(TT_KEYWORD_STATIC),
        ENTRY(TT_KEYWORD_IMPORT),
        ENTRY(TT_KEYWORD_FOR),
        ENTRY(TT_KEYWORD_NUM),
        ENTRY(TT_KEYWORD_Null),

        ENTRY(TT_ASSIGN),
        ENTRY(TT_PLUS),
        ENTRY(TT_MINUS),
        ENTRY(TT_MUL),
        ENTRY(TT_DIV),
        ENTRY(TT_EQ),
        ENTRY(TT_NEQ),
        ENTRY(TT_LT),
        ENTRY(TT_GT),
        ENTRY(TT_LE),
        ENTRY(TT_GE),

        ENTRY(TT_LPAREN),
        ENTRY(TT_RPAREN),
        ENTRY(TT_LBRACE),
        ENTRY(TT_RBRACE),
        ENTRY(TT_COMMA),
        ENTRY(TT_DOT),

        ENTRY(TT_ERROR),
};

static void white_space_remover(char *s)
{
    size_t n = strlen(s);
    while (n && (s[n-1] == '\n' || s[n-1] == '\r')) s[--n] = '\0';
}

static void usage(const char *argv0)
{
    fprintf(stderr, "Usage: ./%s <input_src_code> <test_file>\n", argv0);
}

static int split_tab3(char *line, char **a, char **b, char **c)
{
    *a = line;
    char *p = strchr(line, '\t');
    if (!p) return 0;
    *p++ = '\0';
    *b = p;
    p = strchr(p, '\t');
    if (!p) return 0;
    *p++ = '\0';
    *c = p;
    return 1;
}

int main (int argc, char **argv)
{
    printf("running\n");
    if (argc != 3)
    {
        usage(argv[0]);
        return 2;
    }

    const char *source_path = argv[1];
    const char *golden_path = argv[2];

    FILE *src = freopen(source_path, "r", stdin);
    if(!src) { perror("Couldn't open input file\n"); return 2; }

    FILE *gold = fopen(golden_path, "r");
    if(!gold) { perror("Couldn't open test file\n"); return 2; }

    char line[4096];
    size_t line_no = 0;
    int failures = 0, checks = 0;

    while (fgets(line, sizeof(line), gold))
    {
        ++line_no;
        white_space_remover(line);
        if (line[0] == '\0') continue;

        char *lex_exp, *name_exp, *num_exp_s;
        split_tab3(line, &lex_exp, &name_exp, &num_exp_s);

        char *endp = NULL;
        long enum_exp = strtol(num_exp_s, &endp, 10);

        token_t token = get_token();
        checks++;

        if((long)token.type != enum_exp) {
            fprintf(stderr, "Line %zu: invalid TYPE; exp.: %s(%ld) got: %s(%d)\n",
                    line_no, name_exp, enum_exp, enum_to_name(token.type), (int) token.type);
            failures++;
        }

        fclose(gold);

        if (failures == 0) {
            printf("OK: %d comparisons\n", checks);
            return 0;
        } else {
            fprintf(stderr, "DONE: %d comparisons, %d failures\n", checks, failures);
        }

    }


}
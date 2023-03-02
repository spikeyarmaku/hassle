/*
TODO
- check memory allocations for leaks

IDEAS
Expr:
- After parsing the AST, we could modify it so that every symbol can be indexed
  by a number instead of a string. It could be turned on by a switch (--low-mem)
- Instead of using a linked list, use an array (like with Alints)
*/

#include "main.h"

#ifdef MEMORY_DIAGNOSTIC
struct Logger _logger;
#endif

struct Parser create_parser(char* filename);

#ifdef REPL_ENABLED
void repl() {
    char buffer[BUFFER_SIZE];

    int go_on = 1;
    while (go_on) {
        printf("REAL: ");

        if (!fgets(buffer, BUFFER_SIZE, stdin)) {
            printf("Error while reading from stdin.\n");
        }

        if (buffer[0] == '\n') {
            go_on = 0;
            break;
        }

        struct Expr* expr = parse_from_str(buffer);
        print_expr(expr);
        destroy_expr(expr);
        printf("\n\n");
    }
}
#endif

void interpret_file(char* file_name) {
    printf("%s\n", file_name);
    struct Expr* expr = parse_from_file(file_name);
    print_expr(expr);
    destroy_expr(expr);
}

void assert(int* sum, char* assertion, int x) {
    printf("Testing assertion \"%s\"...", assertion);
    if (!x) {
        printf("FAILED\n");
        (*sum)++;
    } else {
        printf("SUCCESS\n");
    }
}

uint8_t string_compare(char* str1, char* str2) {
    uint8_t res = strcmp(str1, str2);
    free(str1);
    return res;
}

uint8_t run_tests() {
    int sum = 0;
    char buf[BUFFER_SIZE];

    sprintf(buf, "1");
    Alint alint0 = string_to_alint(buf);
    assert(&sum, "alint0", alint0[0] == 1);
    destroy_alint(alint0);

    sprintf(buf, "283,756,238,567");
    Alint alint1 = string_to_alint(buf);
    assert(&sum, "alint1",
        string_compare(debug_print_alint(alint1), "283756238567") == 0);
    destroy_alint(alint1);

    sprintf(buf, "19581958");
    Alint alint_sub_1 = string_to_alint(buf);
    sprintf(buf, "9581958");
    Alint alint_sub_2 = string_to_alint(buf);
    Alint diff = sub_alint(alint_sub_1, alint_sub_2, NULL);
    assert(&sum, "diff",
        string_compare(debug_print_alint(diff), "10000000") == 0);
    destroy_alint(alint_sub_1);
    destroy_alint(alint_sub_2);
    destroy_alint(diff);

    sprintf(buf, "308023895");
    Alint alint2 = string_to_alint(buf);
    sprintf(buf, "350108375");
    Alint alint3 = string_to_alint(buf);
    Alint gcd = gcd_alint(alint2, alint3); // should give 4765
    assert(&sum, "gcd", string_compare(debug_print_alint(gcd), "4765") == 0);
    destroy_alint(alint2);
    destroy_alint(alint3);
    destroy_alint(gcd);

    sprintf(buf, "4.000");
    struct Rational r1 = string_to_rational(buf);
    sprintf(buf, "003");
    struct Rational r2 = string_to_rational(buf);
    struct Rational r3 = add_rational(r1, r2);
    struct Rational r4 = sub_rational(r1, r2);
    struct Rational r5 = mul_rational(r1, r2);
    struct Rational r6 = div_rational(r1, r2);
    assert(&sum, "r1 numer", r1.numerator[0] == 4);  assert(&sum, "r1 denom", r1.denominator[0] == 1);
    assert(&sum, "r2 numer", r2.numerator[0] == 3);  assert(&sum, "r2 denom", r2.denominator[0] == 1);
    assert(&sum, "r3 numer", r3.numerator[0] == 7);  assert(&sum, "r3 denom", r3.denominator[0] == 1);
    assert(&sum, "r4 numer", r4.numerator[0] == 1);  assert(&sum, "r4 denom", r4.denominator[0] == 1);
    assert(&sum, "r5 numer", r5.numerator[0] == 12); assert(&sum, "r5 denom", r5.denominator[0] == 1);
    assert(&sum, "r6 numer", r6.numerator[0] == 4);  assert(&sum, "r6 denom", r6.denominator[0] == 3);
    destroy_rational(r1); destroy_rational(r2); destroy_rational(r3);
    destroy_rational(r4); destroy_rational(r5); destroy_rational(r6);

    printf("%d tests failed.\n", sum);
}

// If called with a file, run it, else start a REPL
int main(int argc, char *argv[]) {
    printf("----------\nRealScript\n----------\n\n");
    init_logger();
    if (argc > 1) {
        // There is at least one parameter
        // interpret_file(argv[1]);

        run_tests();
    } else {
        #ifdef REPL_ENABLED
        // There are no arguments, start a REPL
        repl();
        #else
        printf("Error: no filename provided.\n");
        #endif
    }
    return 0;
}


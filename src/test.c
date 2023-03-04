#include "test.h"

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

uint8_t run_alnat_tests() {
    printf("Running alnat tests...\n");
    int sum = 0;
    char buf[STRING_BUFFER_SIZE];

    sprintf(buf, "1");
    Alnat alnat0 = string_to_alnat(buf);
    assert(&sum, "alnat0", alnat0[0] == 1);
    destroy_alnat(alnat0);

    sprintf(buf, "283,756,238,567");
    Alnat alnat1 = string_to_alnat(buf);
    assert(&sum, "alnat1",
        string_compare(debug_print_alnat(alnat1), "283756238567") == 0);
    destroy_alnat(alnat1);

    sprintf(buf, "19581958");
    Alnat alnat_sub_1 = string_to_alnat(buf);
    sprintf(buf, "9581958");
    Alnat alnat_sub_2 = string_to_alnat(buf);
    Alnat diff = sub_alnat(alnat_sub_1, alnat_sub_2, NULL);
    assert(&sum, "diff",
        string_compare(debug_print_alnat(diff), "10000000") == 0);
    destroy_alnat(alnat_sub_1);
    destroy_alnat(alnat_sub_2);
    destroy_alnat(diff);

    sprintf(buf, "308023895");
    Alnat alnat2 = string_to_alnat(buf);
    sprintf(buf, "350108375");
    Alnat alnat3 = string_to_alnat(buf);
    Alnat gcd = gcd_alnat(alnat2, alnat3); // should give 4765
    assert(&sum, "gcd", string_compare(debug_print_alnat(gcd), "4765") == 0);
    destroy_alnat(alnat2);
    destroy_alnat(alnat3);
    destroy_alnat(gcd);

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
    return sum;
}

uint8_t run_expr_tests() {
    printf("Running expr tests...\n");
    int sum = 0;
    uint8_t error_code = SUCCESS;
    char msg[1000];

    // Parse and compare exprs
    // TODO test comments
    char buf[] =
        "(defun factorial (n) (if (<= n 1) 1 (* n (factorial (- n 1)))))";
    Expr expr;
    struct Dict dict = make_dict();
    error_code = parse_from_str(buf, &expr, &dict);
    if (error_code != SUCCESS) {
        error("couldn't parse string\n");
        return error_code;
    }
    print_expr(expr, dict, msg);
    assert(&sum, "Parse an expression", strcmp(buf, msg) == 0);
    destroy_dict(dict);
    assert(&sum, "Compare two expressions", is_equal_expr(expr, expr) == 1);
    destroy_expr(expr);
    expr = NULL;

    // Find longest match
    char buf2[] = "(a b (x (1 2 3) y) c)";
    char buf3[] = "(a b (x (1 2 3) z) c)";
    Expr expr2; struct Dict dict2 = make_dict();
    error_code = parse_from_str(buf2, &expr2, &dict2);
    if (error_code != SUCCESS) {
        error("couldn't parse string\n");
        return error_code;
    }
    Expr expr3;
    error_code = parse_from_str(buf3, &expr3, &dict2);
    if (error_code != SUCCESS) {
        error("couldn't parse string\n");
        return error_code;
    }
    print_expr(expr2, dict2, msg); printf("expr2: %s\n", msg);
    print_expr(expr3, dict2, msg); printf("expr3: %s\n", msg);
    assert(&sum, "Find longest match", match_size(expr2, expr3) == 2);
    printf("%d tests failed.\n", sum);
    destroy_expr(expr2); destroy_expr(expr3);
    destroy_dict(dict2);
    expr2 = NULL; expr3 = NULL;

    show_logger_entries(_logger);
    return sum;
}

void run_tests() {
    int sum = 0;
    // sum += run_alnat_tests();
    sum += run_expr_tests();

    printf("Total: %d tests failed.\n", sum);
    return;
}

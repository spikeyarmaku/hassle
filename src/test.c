#include "test.h"

void assert(int* sum, char* assertion, int x) {
    printf("Testing assertion \"%s\"...", assertion);
    if (!x) {
        printf("FAILED\n");
        (*sum)++;
    } else {
        printf("Success\n");
    }
}

uint8_t string_compare(char* str1, char* str2) {
    uint8_t res = strcmp(str1, str2);
    free(str1);
    return res;
}

// uint8_t run_number_tests() {
//     printf("Running number tests...\n");
//     int sum = 0;
//     char buf[STRING_BUFFER_SIZE];

//     sprintf(buf, "1");
//     Alnat_t alnat0 = string_to_alnat(buf);
//     assert(&sum, "alnat0", alnat0[0] == 1);
//     alnat_free(alnat0);

//     sprintf(buf, "283,756,238,567");
//     Alnat_t alnat1 = string_to_alnat(buf);
//     assert(&sum, "alnat1",
//         string_compare(debug_print_alnat(alnat1), "283756238567") == 0);
//     alnat_free(alnat1);

//     sprintf(buf, "19581958");
//     Alnat_t alnat_sub_1 = string_to_alnat(buf);
//     sprintf(buf, "9581958");
//     Alnat_t alnat_sub_2 = string_to_alnat(buf);
//     Alnat_t diff = alnat_sub(alnat_sub_1, alnat_sub_2, NULL);
//     assert(&sum, "diff",
//         string_compare(debug_print_alnat(diff), "10000000") == 0);
//     alnat_free(alnat_sub_1);
//     alnat_free(alnat_sub_2);
//     alnat_free(diff);

//     sprintf(buf, "308023895");
//     Alnat_t alnat2 = string_to_alnat(buf);
//     sprintf(buf, "350108375");
//     Alnat_t alnat3 = string_to_alnat(buf);
//     Alnat_t gcd = alnat_gcd(alnat2, alnat3); // should give 4765
//     assert(&sum, "gcd", string_compare(debug_print_alnat(gcd), "4765") == 0);
//     alnat_free(alnat2);
//     alnat_free(alnat3);
//     alnat_free(gcd);

//     sprintf(buf, "4.000");
//     Rational_t r1 = string_to_rational(buf);
//     sprintf(buf, "003");
//     Rational_t r2 = string_to_rational(buf);
//     Rational_t r3 = add_rational(r1, r2);
//     Rational_t r4 = sub_rational(r1, r2);
//     Rational_t r5 = mul_rational(r1, r2);
//     Rational_t r6 = div_rational(r1, r2);
//     assert(&sum, "r1 numer", r1->numerator[0] == 4);
//     assert(&sum, "r1 denom", r1->denominator[0] == 1);
//     assert(&sum, "r1 sign",  r1->sign == 1);
//     assert(&sum, "r2 numer", r2->numerator[0] == 3);
//     assert(&sum, "r2 denom", r2->denominator[0] == 1);
//     assert(&sum, "r2 sign",  r2->sign == 1);
//     assert(&sum, "r3 numer", r3->numerator[0] == 7);
//     assert(&sum, "r3 denom", r3->denominator[0] == 1);
//     assert(&sum, "r3 sign",  r3->sign == 1);
//     assert(&sum, "r4 numer", r4->numerator[0] == 1);
//     assert(&sum, "r4 denom", r4->denominator[0] == 1);
//     assert(&sum, "r4 sign",  r4->sign == 1);
//     assert(&sum, "r5 numer", r5->numerator[0] == 12);
//     assert(&sum, "r5 denom", r5->denominator[0] == 1);
//     assert(&sum, "r5 sign",  r5->sign == 1);
//     assert(&sum, "r6 numer", r6->numerator[0] == 4);
//     assert(&sum, "r6 denom", r6->denominator[0] == 3);
//     assert(&sum, "r6 sign",  r6->sign == 1);
//     free_rational(r1); free_rational(r2); free_rational(r3);
//     free_rational(r4); free_rational(r5); free_rational(r6);

//     printf("%d tests failed.\n\n", sum);
//     return sum;
// }

uint8_t run_expr_tests() {
    printf("Running expr tests...\n");
    int sum = 0;
    uint8_t Error_code = Success;

    // Empty list
    uint8_t expr_buf[] = {OpenParen, CloseParen, Eos};
    Expr_t empty_list = (Expr_t)expr_buf;
    assert(&sum, "Check if an expr is a list",
        expr_is_list(empty_list) == 1);
    
    // Parse and compare exprs
    char buf[] =
        "(defun factorial (n) (if (<= n 1) 1 (* n (factorial (- n 1)))))";
    ErrorCode_t error_code;
    Expr_t expr = parse_from_str(&error_code, buf);
    if (error_code != Success) {
        error("couldn't parse string\n");
        return Error_code;
    }
    char* msg = expr_to_string(expr);
    assert(&sum, "Parse an expression", strcmp(buf, msg) == 0);
    free_mem("run_expr_tests", msg);
    assert(&sum, "Compare two expressions", expr_is_equal(expr, expr) == TRUE);
    expr_free(&expr);
    
    // Find longest match
    char buf2[] = "(a b (x (1 2 3) y) c)";
    char buf3[] = "(a b (x (1 2 3) z) c)";
    Expr_t expr2 = parse_from_str(&error_code, buf2);
    if (error_code != Success) {
        error("couldn't parse string\n");
        return Error_code;
    }
    Expr_t expr3 = parse_from_str(&error_code, buf3);
    if (error_code != Success) {
        error("couldn't parse string\n");
        return Error_code;
    }
    char* msg2 = expr_to_string(expr2); printf("expr2: %s\n", msg2);
    char* msg3 = expr_to_string(expr3); printf("expr3: %s\n", msg3);
    free_mem("run_expr_tests", msg2); free_mem("run_expr_tests", msg3);
    assert(&sum, "Compare two different subexpressions",
        expr_is_equal(expr2 + 1, expr3 + 1) == TRUE);
    // assert(&sum, "Find longest match", match_size(expr2, expr3) == 2);
    printf("%d tests failed.\n\n", sum);
    expr_free(&expr2); expr_free(&expr3);
    
    // show_logger_entries(_logger);
    return sum;
}

// uint8_t run_exec_tests() {
//     printf("Running exec tests...\n");
    
//     EnvFrame_t frame = make_default_frame();
//     char source[] = "(+ (/ 2 (- 8 2)) (* 2 4))";
//     Expr_t expr;
//     enum ErrorCode Error_code = parse_from_str(source, &expr, &(frame->env_dict->symbol_dict));
//     if (Error_code != Success) {
//         Error("couldn't parse %s\n", source);
//         return Error_code;
//     }

//     struct Term result;
//     Error_code = eval_expr(frame, expr, &result);
//     if (Error_code != Success) {
//         return Error_code;
//     }
    
//     switch (result.type) {
//         case AbsTerm: {
//             printf("Result is an abstraction.\n");
//             break;
//         }
//         case ValTerm: {
//             printf("Result is a value ");
//             switch (result.value.type) {
//                 case RationalVal: {
//                     printf("(rational): ");
//                     debug_print_rational(result.value.rational);
//                     printf("\n");
//                     break;
//                 }
//                 case StringVal: {
//                     printf("(string): %s\n", result.value.string);
//                     break;
//                 }
//             }
//             break;
//         }
//         case ExprTerm: {
//             char buf[1024];
//             print_expr(result.expr, frame, buf);
//             printf("Result is an expression: %s\n", buf);
//             break;
//         }
//     }

//     free_term(result);
//     free_frame(&frame);
//     free_expr(&expr);

//     show_logger_entries(_logger);
//     return 0;
// }

void run_tests() {
    int sum = 0;
    // sum += run_number_tests();
    sum += run_expr_tests();
    // sum += run_exec_tests();

    printf("Total: %d tests failed.\n", sum);
    return;
}

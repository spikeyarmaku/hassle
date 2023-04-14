#ifndef _TEST_H_
#define _TEST_H_

#include <stdio.h>

#include "config.h"
#include "global.h"

#include "parse\parse.h"
#include "rational\rational.h"
// #include "execute\eval.h"
// #include "execute\builtin.h"

#include "memory.h"
#include "test.h"

void test_assert(int*, char*, int);
uint8_t string_compare(char*, char*);
uint8_t run_alnat_tests();
uint8_t run_expr_tests();
uint8_t run_exec_tests();
void run_tests();

#endif
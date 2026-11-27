// expect: rejected cerastes-unsupported_type @ L21
// expect: rejected cerastes-unsupported_type @ L26
// expect: rejected cerastes-unsupported_type @ L35
// expect: rejected cerastes-unsupported_type @ L43
// expect: rejected cerastes-unsupported_type @ L47
// mode: c23, c++20
// twin: tests/memory/matrix_value_limit_seven.c
// Pins: 0064 item 12 (question 353, as amended on 2026-11-26; M4.5f3): 8 counted cells is rejected for a by-value parameter and result (at the function's line), a compound literal assigned to a structure and a whole-structure copy (at the expression; the initializer form `struct V8 v = (struct V8){...}` is the C-only file matrix_value_limit_eight_literal_init.c: C++ reads it as a braced initialization and accepts it), struct V8 { int m[2][4]; }; and the row that tells the two counts apart: struct T { int m[2][3]; int a[2]; } by value counts 6 + 2 = 8 and is rejected as a parameter, where its copy (matrix_value_limit_seven.c) counts 6 and is accepted
// Pre-registered from record 0064 item 12 (Part E), before any run: rejected (unsupported_type at each line, by the encoder's messages for the value and the copy limits; to confirm at the first run); edited by test-mutator-11 (2026-11-27, before any run of slice 4) after the encoder owner's probe showed that the first draft's L34, the INITIALIZER `struct V8 v = (struct V8){...}`, is accepted in C++ (a braced initialization builds no value): the row is now the assignment form, which the probe showed rejected in both modes, and the initializer form moved to the C-only file; every later row is one line lower
#include <cerastes.h>

struct V8 {
  int m[2][4];
};

struct T {
  int m[2][3];
  int a[2];
};

int param8(struct V8 v)  // <- here
{
  return v.m[0][0] * 0;
}

struct V8 result8(void)  // <- here
{
  struct V8 v = {{{0, 0, 0, 0}, {0, 0, 0, 0}}};
  return v;
}

int literal8(void)
{
  struct V8 v = {{{0, 0, 0, 0}, {0, 0, 0, 0}}};
  v = (struct V8){{{0, 0, 0, 0}, {0, 0, 0, 0}}};  // <- here
  return v.m[0][0] * 0;
}

int copy8(void)
{
  struct V8 a = {{{1, 2, 3, 4}, {5, 6, 7, 8}}};
  struct V8 b = {{{0, 0, 0, 0}, {0, 0, 0, 0}}};
  b = a;  // <- here
  return b.m[1][3] * 0;
}

int param_t(struct T t)  // <- here
{
  return t.m[1][2] * 0;
}

// expect: rejected cerastes-unsupported_type @ L15
// mode: c23
// c-only: in C++ the same line is a braced initialization and is accepted (record 0064 item 12): the initializer form of a compound literal is a structure value in C only
// twin: tests/memory/matrix_value_limit_seven_copy.c
// Pins: 0064 item 12 (question 353; record item 12's last sentence, "in C++ `struct T t = (struct T){...};` is the initializer's braced list"): in C the initializer `struct V8 v = (struct V8){...};` reads the compound literal as a structure value of 8 counted cells (struct V8 { int m[2][4]; }) and is rejected at its line with the 7-cell limit; the same line in C++ is a braced initialization (no value is built) and is accepted, so this file is C only; the assignment form, rejected in both modes, is in matrix_value_limit_eight.c
// Pre-registered from record 0064 item 12 (Part E), before any run (test-mutator-11, 2026-11-27): rejected (unsupported_type at the line, the first draft's L34 of matrix_value_limit_eight.c, which the encoder owner's probe measured in C23: rejected; to confirm at the first run)
#include <cerastes.h>

struct V8 {
  int m[2][4];
};

int literal8_init(void)
{
  struct V8 v = (struct V8){{{0, 0, 0, 0}, {0, 0, 0, 0}}};  // <- here
  return v.m[0][0] * 0;
}

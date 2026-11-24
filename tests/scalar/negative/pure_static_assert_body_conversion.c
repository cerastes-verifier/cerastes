// expect: error conversion @ L13
// mode: c23, c++20
// twin: tests/scalar/pure_static_assert_body.c
// Pins: 0062 item 3 and 0024 item 2: a block-scope static_assert in a read pure body whose condition converts out of range is the conversion error, at the assertion (the tag and line the same assertion gets in an ordinary function: static_assert_block_conversion.c); the region of a read pure function is no longer exempt from the cross-check
// Pre-registered from record 0062 (Part B), before the code: conversion. the line is a question-160 candidate (the assertion's own line, as in an ordinary function)
#include <cerastes.h>

CERASTES_PURE int widen(int x)
    CERASTES_REQUIRES(0 <= x && x <= 1000)
    CERASTES_ENSURES(r, r == x + 4)
{
  static_assert(sizeof(int) == 4, "int is 32 bits");
  static_assert((signed char)200 == -56, "a cast that wraps");
  return x + (int)sizeof(int);
}

int use(int y)
    CERASTES_REQUIRES(0 <= y && y <= 1000)
{
  CERASTES_CHECK(widen(y) == y + 4);
  return y;
}

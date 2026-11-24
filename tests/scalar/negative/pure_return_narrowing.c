// expect: error conversion @ L10
// mode: c23, c++20
// twin: tests/scalar/pure_return_narrowing_ok.c
// Pins: 0062 item 9: a value that may not fit the result type at a pure function's return is the conversion error, at the conversion, in the function
// Pre-registered from record 0062 (Part B), before the code: conversion
#include <cerastes.h>

CERASTES_PURE short narrow(int x)
{
  return (short)x;
}

int use(int y)
    CERASTES_REQUIRES(-50 < y && y < 50)
{
  CERASTES_CHECK(narrow(y) == narrow(y));
  return y;
}

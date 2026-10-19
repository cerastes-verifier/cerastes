// expect: rejected cerastes-unsupported_expression @ L10
// c-only: ++ on bool is ill-formed in C++17 and later
// twin: tests/scalar/if_else.c
// Pins: 0012 item 7: ++ on bool is rejected (C only: C++ removed it)
#include <cerastes.h>

int flip(int a)
{
  bool b = false;
  b++;
  return a + b;
}

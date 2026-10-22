// expect: rejected cerastes-unsupported_expression @ L10
// mode: c23, c++20
// twin: tests/scalar/const_global.c
// Pins: 0017 item 2: a global declared in a system file is rejected at its use (a value is not a contract)
#include <stdio.h>
#include <cerastes.h>

int count(void)
{
  return sys_nerr;
}

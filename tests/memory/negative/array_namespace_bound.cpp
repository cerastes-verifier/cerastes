// expect: error overflow @ L8
// mode: c++20
// twin: tests/memory/array_sum_two.c
// Pins: 0059 item 2 (audit round 1, item 4): a namespace-scope const keeps expr: var N, so the bound goes through 0017's named constants, checked in the prologue as written: 65536u * 65536u + 1 overflows unsigned int whether or not an array uses it (the same error, at the same line, as for return (int)N;)
// Pre-registered from a run of build/cerastes verify on the same constant without an array (M4.5e1a, report 4): overflow at the constant's line
#include <cerastes.h>

const unsigned N = 65536u * 65536u + 1;

int f(void)
{
  int a[N];
  a[0] = 1;
  return a[0];
}

// expect: error overflow @ L10
// mode: c23, c++20
// twin: tests/memory/array_sum_two.c
// Pins: 0059 item 2 and 0024: a bound is read from its written expression and checked as written: 65536u * 65536u + 1 overflows unsigned int (cerastes does not wrap), though the front end wraps it to one element
// Pre-registered from the coordinator's report 3 (edg-backend's analogy to 0024; to be confirmed by the first run of the lowering)
#include <cerastes.h>

int f(void)
{
  int a[65536u * 65536u + 1];
  a[0] = 1;
  return a[0];
}

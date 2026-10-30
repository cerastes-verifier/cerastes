// expect: rejected cerastes-unsequenced_modification @ L14
// mode: c23, c++20
// twin: tests/memory/ptrptr_comma_sequenced.c
// Pins: 0040 item 2 (P62 A8, 0020 item 5's union rule): *pp == (*pp = &y) names *pp or **pp twice with a write; GCC's -Wall warns (clang is silent): rejected like the compilers (never lifted)
#include <cerastes.h>

int shape(void)
{
  int x = 0;
  int y = 0;
  int r = 0;
  int *p = &x;
  int **pp = &p;
  r = (*pp == (*pp = &y));
  return r + x + y;
}

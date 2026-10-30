// expect: rejected incompatible_param @ L18
// mode: c++20
// twin: tests/memory/ptrptr_const_middle.c
// Pins: 0040 item 2 (P62 H1): the same conversion of an argument (#167)
#include <cerastes.h>
#include <stddef.h>

int take(const int **q)
{
  return q == nullptr;
}

int laundering_arg(void)
{
  int x = 0;
  int *p = &x;
  int **pp = &p;
  return take(pp);
}

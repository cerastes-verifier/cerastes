// expect: rejected cerastes-c99_designator @ L12
// expect: rejected cerastes-c99_designator @ L18
// expect: rejected cerastes-c99_designator @ L24
// mode: c++20
// twin: tests/memory/array_init_partial.c
// Pins: 0059 item 3 (audit round 1): in C++ every array designator is c99_designator, whatever its index: a wrapping index, the bound of an unsized array and an index given twice included
// Pre-registered from record 0059 item 3 as updated by audit round 1
#include <cerastes.h>

int wrapped(void)
{
  int b[3] = {[65536u * 65536u + 1] = 7};
  return b[1];
}

int unsized(void)
{
  int c[] = {[65536u * 65536u] = 9};
  return c[0];
}

int twice(void)
{
  int d[2] = {[0] = 1, [0] = 2};
  return d[0];
}

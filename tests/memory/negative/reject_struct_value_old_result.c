// expect: rejected cerastes-unsupported_annotation @ L13
// mode: c23, c++20
// twin: tests/memory/struct_value_result.c
// Pins: 0044 item 4: old(r.x) of the result: a result has no entry value
#include <cerastes.h>

struct S {
  int x;
  int y;
};

struct S f(void)
    CERASTES_ENSURES(r, cerastes_old(r.x) == 1)  // <- here
{
  struct S s = {1, 2};
  return s;
}

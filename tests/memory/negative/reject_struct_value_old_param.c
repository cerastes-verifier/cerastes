// expect: rejected cerastes-unsupported_annotation @ L14
// mode: c23, c++20
// twin: tests/memory/struct_value_param.c
// Pins: 0044 item 3: old(s) of a by-value parameter is a whole struct value in a contract: rejected (old(s.x) is the entry value)
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int f(struct S s)
    CERASTES_REQUIRES(s.x == 1)
    CERASTES_ENSURES(r, cerastes_old(s).x == 1)  // <- here
{
  return s.x;
}

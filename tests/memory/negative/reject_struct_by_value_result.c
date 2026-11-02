// expect: rejected cerastes-unsupported_type @ L12
// mode: c23, c++20
// twin: tests/memory/struct_local.c
// Pins: 0043 items 8 and 15: a by-value struct result is rejected (M4.5c2)
#include <cerastes.h>

struct S {
  int x;
  int y;
};

struct S make(void) { struct S s = {1, 2}; return s; }  // <- here

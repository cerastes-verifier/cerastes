// expect: rejected cerastes-unsupported_annotation @ L9
// mode: c23, c++20
// twin: tests/memory/owning_oneline_match.c
// Pins: 0040 item 3 (audit round 2 S2): two declarations of f from one macro expansion have one position, so cerastes cannot tell their owning apart: rejected
#include <cerastes.h>
#include <stddef.h>

#define DECL_AND_DEF void f(int *p); void f(CERASTES_OWNING int *p) { if (p != NULL) *p = 1; }
DECL_AND_DEF

int g(void)
{
  int x = 0;
  f(&x);
  return x;
}

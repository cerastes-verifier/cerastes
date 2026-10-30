// expect: rejected cerastes-annotation_mismatch @ L8
// mode: c23, c++20
// twin: tests/memory/owning_oneline_match.c
// Pins: 0040 item 3 (audit round 2 S2): the prototype lacks owning and the definition has it, both on one line: annotation_mismatch
#include <cerastes.h>
#include <stddef.h>

void f(int *p); void f(CERASTES_OWNING int *p) { if (p != NULL) *p = 1; }

int g(void)
{
  int x = 0;
  f(&x);
  f(NULL);
  return x;
}

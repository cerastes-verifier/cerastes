// expect: rejected cerastes-unsupported_annotation @ L12
// mode: c23, c++20
// twin: tests/memory/result_bodiless.c
// Pins: 0052 item 2a (audit B3): a pointer read through the result equated with a formal (r->q == p) in a bodiless contract: the result is no longer a place the caller does not hold
// Pre-registered from record 0052 item 2a: rejected unsupported_annotation at the declaration
#include <cerastes.h>

struct N {
  int *q;
};

struct N *mk(int *p) CERASTES_ENSURES(r, cerastes_owns(*r) && r->q == p);  // <- here

int main(void)
{
  int x = 0;
  struct N *n = mk(&x);
  (void)n;
  return 0;
}

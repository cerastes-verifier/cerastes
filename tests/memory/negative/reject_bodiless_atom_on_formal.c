// expect: rejected cerastes-unsupported_annotation @ L8
// mode: c23, c++20
// twin: tests/memory/result_bodiless.c
// Pins: 0052 item 2a (audit B3, e9): a partnerless atom on a formal in a bodiless contract would give the caller a second write chunk on a cell it already holds, and everything after the call would verify: rejected, naming M6's contract manifest
// Pre-registered from record 0052 item 2a: rejected unsupported_annotation at the declaration
#include <cerastes.h>

void f(int *p) CERASTES_REQUIRES(cerastes_live(p)) CERASTES_ENSURES(cerastes_owns(*p));  // <- here

int main(void)
{
  int x = 0;
  f(&x);
  return 0;
}

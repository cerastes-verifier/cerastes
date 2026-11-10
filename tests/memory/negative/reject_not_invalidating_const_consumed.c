// expect: rejected cerastes-unsupported_annotation @ L8
// mode: c23, c++20
// twin: tests/memory/not_invalidating_const.c
// Pins: 0058 item 2 (question 203): a marked pointer to a const object whose contract takes reads(*p) and does not return it (today's encoder lets the callee keep the share: the caller cannot write x afterwards): unsupported_annotation at the parameter
// Pre-registered from record 0058 item 2: rejected unsupported_annotation at the parameter; a reads atom with no ensures partner is taken, as a caller's write after the call shows today (build/m4.5d2b2d/emul/f4.c: permission "a callee may have kept its object")
#include <cerastes.h>

int zero_of(const int *p CERASTES_NOT_INVALIDATING)  // <- here
    CERASTES_REQUIRES(cerastes_reads(*p))
    CERASTES_ENSURES(r, r == 0)
{
  return 0;
}

int use(void)
    CERASTES_ENSURES(r, r == 0)
{
  int x = 4;
  int v = zero_of(&x);
  x = 5;
  return v;
}

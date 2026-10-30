// expect: rejected cerastes-unsupported_annotation @ L10
// expect: rejected cerastes-unsupported_annotation @ L11
// mode: c23, c++20
// twin: tests/memory/owns_inc.c
// Pins: 0040 item 2 (re-audit of audit round 2's B1): owns of a const place (*v of a const int * parameter) would let the caller's const object be written through the contract; rejected
#include <cerastes.h>
#include <stddef.h>

void poke(const int *v)
    CERASTES_REQUIRES(cerastes_owns(*v))
    CERASTES_ENSURES(cerastes_owns(*v) && *v == 2);

int f(void)
{
  const int c = 1;
  poke(&c);
  CERASTES_CHECK(c == 2);
  return 0;
}

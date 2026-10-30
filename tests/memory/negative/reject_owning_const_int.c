// expect: rejected cerastes-unsupported_annotation @ L8
// mode: c23, c++20
// twin: tests/memory/owning_param.c
// Pins: 0040 item 2 (re-audit of audit round 2's B1): owning on a const int * parameter owns a const place: rejected
#include <cerastes.h>
#include <stddef.h>

void poke(CERASTES_OWNING const int *v)
    CERASTES_ENSURES(v == NULL || *v == 2);

int f(void)
{
  const int c = 1;
  poke(&c);
  CERASTES_CHECK(c == 2);
  return 0;
}

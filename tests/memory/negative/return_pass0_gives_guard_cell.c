// expect: error permission @ L17
// mode: c23, c++20
// twin: tests/memory/return_pass0_keeps.c
// Pins: 0045 item 6 (pass 0, audit round 2 S3): the callee takes *c, the cell the guard of owns(*q) reads, so the return's guard read is not defined: permission at the return, 'returns without owning `*c`'
#include <cerastes.h>
#include <stddef.h>

void keepi(int *c)
    CERASTES_REQUIRES(cerastes_owns(*c));

void g(int *c, int *p, [[maybe_unused]] int *q)
    CERASTES_REQUIRES(cerastes_owns(*c) && cerastes_owns(*p) && (*c == 0 || cerastes_owns(*q)))
    CERASTES_ENSURES(cerastes_owns(*c) && *c == cerastes_old(*c) && cerastes_owns(*p) && (*c == 0 || cerastes_owns(*q)))
{
  keepi(c);
  (void)p;
}  // <- here

// expect: error permission @ L10
// mode: c23, c++20
// twin: tests/memory/conditional_ownership_trusted.c
// Pins: 0039 item 7, partners by path (question 72, audit S7): requires(c ? owns(*p) : live(p)) with ensures(owns(*p)): the ensures atom holds on the path c == 0 where only liveness was taken: permission at the ensures atom
#include <cerastes.h>
#include <stddef.h>

void upgrade(int *p, int c)
    CERASTES_REQUIRES(c ? cerastes_owns(*p) : cerastes_live(p))
    CERASTES_ENSURES(cerastes_owns(*p));

void h(int *p)
    CERASTES_REQUIRES(cerastes_live(p))
{
  upgrade(p, 0);
}

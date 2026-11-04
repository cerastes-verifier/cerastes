// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0045 item 6 (pass 0): a callee that keeps what its contract promises on return, for a place reached through a load (f) and for a guard that reads the heap (g, whose `*c == old(*c)` makes the entry and return guards one); the twins give the cell away to a callee and fail at the return
#include <cerastes.h>
#include <stddef.h>

void f(int **pp)
    CERASTES_REQUIRES(cerastes_owns(*pp) && cerastes_owns(**pp))
    CERASTES_ENSURES(cerastes_owns(*pp) && cerastes_owns(**pp))
{
  (void)pp;
}

void g(int *c, int *p, [[maybe_unused]] int *q)
    CERASTES_REQUIRES(cerastes_owns(*c) && cerastes_owns(*p) && (*c == 0 || cerastes_owns(*q)))
    CERASTES_ENSURES(cerastes_owns(*c) && *c == cerastes_old(*c) && cerastes_owns(*p) && (*c == 0 || cerastes_owns(*q)))
{
  (void)c;
  (void)p;
}

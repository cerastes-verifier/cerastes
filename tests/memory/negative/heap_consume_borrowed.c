// expect: error permission @ L23
// mode: c23, c++20
// twin: tests/memory/heap_consume.c
// Pins: 0047 item 4: a function that owns the cell but not the allocation cannot give the block away: the call's owns_block needs the token the caller does not hold
// Pre-registered from record 0047 item 4: permission at the call (a call needs a permission the caller does not hold)
#include <cerastes.h>
#include <stdlib.h>

struct account {
  int balance;
};

void dispose(struct account *a)
    CERASTES_REQUIRES(cerastes_owns_block(a))
{
  free(a);
}

void pass_borrowed(struct account *a)
    CERASTES_REQUIRES(cerastes_owns(*a))
    CERASTES_ENSURES(cerastes_owns(*a))
{
  dispose(a);  // <- here
}

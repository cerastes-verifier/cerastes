// expect: error invalid-free @ L29
// mode: c23, c++20
// verify-options: --strict-init
// twin: tests/memory/heap_strict_init_free.c
// Pins: 0047 items 6, 8: the exemption of free does not exempt it from its free rows
// Pre-registered from record 0047 item 6: invalid-free at the second free under --strict-init; re-registered after audit round 1 (record 0047 item 8): the twin allocates with calloc too
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

int make_zeroed(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  struct account *a = (struct account *)calloc(1, sizeof(struct account));
  if (a == NULL) {
    return -1;
  }
  int b = a->balance;
  free(a);
  free(a);  // <- here
  return b;
}

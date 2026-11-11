// expect: error invalid-free @ L35
// mode: c23, c++20
// verify-options: --strict-owner
// twin: tests/memory/strict_owner_p6.c
// Pins: 0058 item 3, audit S7 ("accepted although wrong"): a callee that consumes its block on some paths only (its ensures returns the block where c != 0): param_consumes_block is false for a guarded site, so p6 never looks at the argument; the alias t of the owner v is passed and the default mode reports the free of v afterwards
// Pre-registered from record 0058 item 3 (audit round 1, S7): one invalid-free at free(v) under --strict-owner
#include <cerastes.h>
#include <stdlib.h>

struct account {
  int balance;
};

CERASTES_OWNING struct account *make_account(void)
{
  CERASTES_OWNING struct account *a = (struct account *)malloc(sizeof(struct account));
  if (a != NULL) {
    a->balance = 0;
  }
  return a;
}

void maybe_take(struct account *a, int c)
    CERASTES_REQUIRES(cerastes_owns_block(a))
    CERASTES_ENSURES(c != 0 ? cerastes_owns_block(a) : 1);

int f(int c)
{
  CERASTES_OWNING struct account *v = make_account();
  if (v == NULL) {
    return -1;
  }
  struct account *t = v;
  maybe_take(t, c);
  free(v);  // <- here
  return 0;
}

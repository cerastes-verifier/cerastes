// expect: error leak @ L19
// mode: c23, c++20
// verify-options: --strict-owner
// twin: tests/memory/strict_owner_p6.c
// Pins: 0058 item 3, audit S7 ("accepted although wrong"): an owner parameter reassigned before it is freed: free(a) of a consuming parameter passes p6 whatever a holds now, and the default mode reports the block a was given and no longer names
// Pre-registered from record 0058 item 3 (audit round 1, S7): the errors of the default mode under --strict-owner; the exact set is read from the first run (question 160)
#include <cerastes.h>
#include <stdlib.h>

struct account {
  int balance;
};

void f(struct account *a, struct account *b)
    CERASTES_REQUIRES(cerastes_owns_block(a) && cerastes_owns_block(b))
{
  a = b;
  free(a);
}  // <- here

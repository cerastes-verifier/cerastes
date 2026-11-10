// expect: rejected cerastes-owner_release_unowned @ L12
// mode: c23, c++20
// verify-options: --strict-owner
// twin: tests/memory/strict_owner_p6.c
// Pins: 0058 item 3 (p6): free of a parameter whose own contract does not take its block (owning on a parameter stays a borrow, question 175): owner_release_unowned at the free (the default mode reports invalid-free)
// Pre-registered from record 0058 item 3: rejected owner_release_unowned at the call of free
#include <cerastes.h>
#include <stdlib.h>

void f(CERASTES_OWNING int *p)
{
  free(p);  // <- here
}

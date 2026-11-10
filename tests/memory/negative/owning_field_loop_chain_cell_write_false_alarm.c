// expect: error invalid-free @ L48
// expect: error invalid-free @ L48
// expect: error invalid-free @ L48
// expect: error leak @ L55
// expect: error permission @ L55
// expect: error permission @ L55
// expect: error invalid-free @ L56
// expect: error invalid-free @ L56
// expect: error invalid-free @ L56
// expect: error invalid-free @ L57
// mode: c23, c++20
// twin: tests/memory/owning_field_loop_chain.c
// Pins: 0056 item 6 (a false alarm pinned, the program is correct; audit round 2 B2, owner question 212: kept as built, M4.8): the loop of owning_field_loop_chain.c plus an unrelated address-taken local `int z` (`keep(&z)` before the loop) written by name in the body (`z = i;`): the places test of item 6 gives a site's cells no places where the loop writes an address-taken local by name or the site's pointer, so the loop's free of a.b->x is not backed by anything it holds (invalid-free) and the rows at the back edge and the return follow; the base verified the first and reported one error for the second (the old struct-local special case gave the cells places under the live guard)
// Pre-registered from the L-2 build's own errors (build/m4.5d2b2c/cerastes-L2, audit probes p5 and p6; a pin records what the encoder reports): the expect lines are its outcome in both modes; M4.8 revisits the places test and re-expects this test
#include <cerastes.h>
#include <stdlib.h>

struct leaf {
  int *x CERASTES_OWNING;
  int k;
};

struct mid {
  struct leaf *b CERASTES_OWNING;
  int m;
};

void keep(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p) && *p == cerastes_old(*p))
{
  (void)p;
}

int f(void)
{
  struct mid a = { NULL, 0 };
  a.b = (struct leaf *)calloc(1, sizeof(struct leaf));
  if (a.b == NULL) {
    return 0;
  }
  int z = 0;
  keep(&z);
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    free(a.b->x);
    a.b->x = (int *)malloc(sizeof(int));
    if (a.b->x != NULL) {
      *a.b->x = i;
    }
    z = i;
    i = i + 1;
  }
  free(a.b->x);
  free(a.b);
  a.b = NULL;
  return z;
}

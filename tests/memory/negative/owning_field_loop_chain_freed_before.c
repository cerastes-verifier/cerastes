// expect: error permission @ L28
// mode: c23, c++20
// twin: tests/memory/owning_field_loop_chain.c
// Pins: 0056 item 7, audit round 2 B1: the same at depth two (a.b freed before the loop, a.b->x freed and replaced in it): the entry row's mapped permission at the loop, and never an unmapped exit 3 (which the base already reports for this shape, record 0056 item 7's second bullet)
// Pre-registered from the shape of p1 and p2 (probe p1b_chain_mid_freed_before.c: exit 3 on the base and on L-2 today, no mapped error observed): permission at the `while` line, in both modes, exit 1; question 160 applies to the line and the category within permission/leak/lifetime, and an exit 3 is a stop [Corrected after the first run on the build with 208-211, 213 (question 160): in c++20 an error of the loop's entry row is reported at the first attribute line of the loop, in c23 at the `while`, so the attributes and the `while` now stand on one line (every later line moved up by two).]
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

int f(void)
{
  struct mid a = { NULL, 0 };
  a.b = (struct leaf *)calloc(1, sizeof(struct leaf));
  if (a.b == NULL) {
    return 0;
  }
  free(a.b);
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3) CERASTES_DECREASES(3 - i) while (i < 3) {
    free(a.b->x);
    a.b->x = (int *)malloc(sizeof(int));
    if (a.b->x != NULL) {
      *a.b->x = i;
    }
    i = i + 1;
  }
  a.b = NULL;
  return 0;
}

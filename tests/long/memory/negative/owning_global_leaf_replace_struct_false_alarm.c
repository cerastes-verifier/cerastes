// expect: error invalid-free @ L47
// expect: error invalid-free @ L47
// expect: error invalid-free @ L47
// expect: error uninitialized @ L47
// expect: error lifetime @ L53
// expect: error invalid-free @ L54
// expect: error leak @ L60
// expect: error permission @ L62
// expect: error permission @ L62
// expect: error permission @ L62
// expect: error permission @ L62
// expect: error leak @ L63
// expect: error permission @ L63
// expect: error permission @ L63
// Record 0058's round-2 B1 reorder leaves one fewer duplicate `permission` at this line (owner question 220); the line was: expect: error permission @ L63
// expect: error uninitialized @ L63
// expect: error uninitialized @ L63
// mode: c23, c++20
// twin: tests/memory/owning_field_loop_chain.c
// Pins: 0056 item 6 (S2, question 213: the replace test of a struct global's leaf; a false alarm pinned, the program is correct): a loop that frees ga.b->x, frees the owning leaf ga.b of the struct global and replaces it by a structure copy through a struct pointer (`*pga = t;` with `pga = &ga`): a closure of ga.b->x carried across the replacement would be unsound, so none is carried and the free of ga.b->x is reported (invalid-free), as the base does Pre-registered from record 0056 item 6 and audit round 2 S2: the expect lines are the base's outcome (build/m4.5d2b2c/cerastes-base, which has no closure for ga at all), which the build with question 213's routes must reproduce; Part L-2 carries the closure across the replacement today and reports fewer errors (the audit's p4) Moved to tests/long/ in M4.5d2b2c (record 0057 item 5, owner question 217: 660 s or more measured on the x86-64 box)
// timeout: 1320
#include <cerastes.h>
#include <stdlib.h>

struct mid {
  int *x CERASTES_OWNING;
  int k;
};

struct top {
  struct mid *b CERASTES_OWNING;
  int m;
};

struct top ga;

void churn(void)
    CERASTES_REQUIRES(cerastes_owns(ga))
    CERASTES_ENSURES(cerastes_owns(ga))
{
  struct top *pga = &ga;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    if (ga.b != NULL) {
      free(ga.b->x);
      ga.b->x = NULL;
    }
    struct mid *n = (struct mid *)malloc(sizeof(struct mid));
    if (n != NULL) {
      n->k = 0;
      if (ga.b != NULL) {
        free(ga.b);
      }
      struct top t;
      t.m = i;
      t.b = n;
      *pga = t;
    }
    i = i + 1;
  }
}

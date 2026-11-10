// expect: error invalid-free @ L45
// expect: error null-dereference @ L45
// expect: error leak @ L51
// expect: error invalid-free @ L52
// expect: error null-dereference @ L52
// mode: c23, c++20
// twin: tests/memory/owning_field_loop_chain.c
// Pins: 0055 item 7 (a false alarm pinned): a loop that frees and replaces a.b->c->x, three owning levels below a struct local: the loop carries the chain to depth 1 only (owning_field_loop_chain.c verifies), so the deeper level's block is not backed by the loop's places. The expectations are the errors the encoder reports today
// Pre-registered from the final re-audit (probes r4/ and r5/), observed on the session build after the fixes: chain3.c: the expect lines are the actual outcome, recorded as a pin
#include <cerastes.h>
#include <stdlib.h>

struct leaf {
  int *x CERASTES_OWNING;
  int k;
};

struct mid {
  struct leaf *c CERASTES_OWNING;
  int j;
};

struct top {
  struct mid *b CERASTES_OWNING;
  int m;
};

int f(void)
{
  struct top a = { NULL, 0 };
  a.b = (struct mid *)calloc(1, sizeof(struct mid));
  if (a.b == NULL) {
    return 0;
  }
  a.b->c = (struct leaf *)calloc(1, sizeof(struct leaf));
  if (a.b->c == NULL) {
    free(a.b);
    a.b = NULL;
    return 0;
  }
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    free(a.b->c->x);
    a.b->c->x = (int *)malloc(sizeof(int));
    if (a.b->c->x != NULL) {
      *a.b->c->x = i;
    }
    i = i + 1;
  }
  free(a.b->c->x);
  free(a.b->c);
  free(a.b);
  a.b = NULL;
  return 0;
}

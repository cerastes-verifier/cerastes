// expect: error leak @ L46
// mode: c23, c++20
// twin: tests/long/memory/owning_field_loop_chain3.c
// Pins: 0056 item 7 (limit 6): the same loop without free(a.b->c->x): the third level's old block is dropped at the end of an iteration, which the loop's back-edge row reports at the loop body's closing brace Pre-registered from record 0056 item 7 and S41 (b) (the twin of the hand-edited fixture, not yet run on the L-2 build): leak at the closing brace of the loop body, in both modes Moved to tests/long/ in M4.5d2b2c (record 0057 item 5, owner question 217: 270 s or more measured on the x86-64 box)
// timeout: 540
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

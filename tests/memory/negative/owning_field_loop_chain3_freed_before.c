// expect: error permission @ L39
// mode: c23, c++20
// twin: tests/long/memory/owning_field_loop_chain3.c
// Pins: 0056 item 7, audit round 2 B1: a three-level chain loop (a.b->c->x) whose middle block a.b->c was freed before the loop, the pointer left dangling: the loop's entry row reports the invariant that needs to own the whole allocation a.b->c points to (mapped permission at the loop), and the carried sites' entry ghosts and case splits, which read through the freed block, must not run before that row: an unmapped Silicon if.failed (exit 3) is the defect
// Pre-registered from audit round 2 B1 (probe p1_chain3_mid_freed_before.c): the base's mapped error, permission at the `while` line, in both modes, exit 1; Part L-2 reports an unmapped exit 3 there until question 211's fix; not yet run on the build with it [Corrected after the first run on the build with 208-211, 213 (question 160): in c++20 an error of the loop's entry row is reported at the first attribute line of the loop, in c23 at the `while`, so the attributes and the `while` now stand on one line (every later line moved up by two).]
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
  free(a.b->c);
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3) CERASTES_DECREASES(3 - i) while (i < 3) {
    free(a.b->c->x);
    a.b->c->x = (int *)malloc(sizeof(int));
    if (a.b->c->x != NULL) {
      *a.b->c->x = i;
    }
    i = i + 1;
  }
  a.b->c = NULL;
  free(a.b);
  a.b = NULL;
  return 0;
}

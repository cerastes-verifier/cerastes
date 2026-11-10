// expect: error permission @ L48
// expect: error leak @ L49
// mode: c23, c++20
// twin: tests/long/memory/owning_field_loop_chain3.c
// Pins: 0056 items 6 and 7, audit round 1 S1: a loop over a three-level chain a.b->c->x below a struct local (the middle block a.b->c is freed) that frees the block the carried sites hang from in one iteration (i == 1): the carried sites read through a freed block at the back edge, which must be a mapped error at the loop and never an exit 3 (the unmapped Silicon `assignment.failed` of a pre-read placed before the rows of the site it reads through) Pre-registered from audit round 1 S1 (not yet run on the L-2 build; the base build exits 3 on the depth-2 form): a mapped permission error at the closing brace of the loop body, in both modes, exit status 1; question 160 applies to the line and the category within permission/leak/lifetime, and an exit 3 is a stop [Corrected after the first run on the L-2 build (question 160): the designed permission is there at the same line; the program's one designed mistake also leaves `a.b` owned at the return on the paths where the loop did not free it, so the encoder reports the leak of the owning field `a.b` there as well.] Moved to tests/long/ in M4.5d2b2c (record 0057 item 5, owner question 217: 180 s or more measured on the x86-64 box)
// timeout: 360
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
    if (i == 1) {
      free(a.b->c->x);
      a.b->c->x = NULL;
      free(a.b->c);
    }
    i = i + 1;
  }
  return 0;
}

// expect: verified
// expect: native-ok
// mode: c23, c++20
// Covers: 0056 item 7 (limit 6): a loop that frees and replaces a.b->c->x, three owning levels below a struct local: the loop carries the chain at any depth, each intermediate cell's flag in the guard of the deeper site
// Pre-registered from record 0056 item 7 and S41 (b) (not yet run on the L-2 build): verifies in both modes; it is the former pin owning_field_loop_chain3_false_alarm.c (invalid-free, null-dereference and leak on the base) with a reworded header
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

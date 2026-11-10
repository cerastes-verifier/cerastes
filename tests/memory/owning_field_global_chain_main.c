// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0055 items 3, 5 (S4, question 174): main ends while the owning leaf g.b of a struct global holds a structure whose own owning field g.b->x holds a block: both blocks are exempt as a static owner's (a chain under a static owner)
// Pre-registered from audit round 1 (record 0055 as revised, 78838ca), verifies in both modes (new: no base form for the chain exemption)
#include <cerastes.h>
#include <stdlib.h>

struct leaf {
  int *x CERASTES_OWNING;
  int *y;
};

struct mid {
  struct leaf *b CERASTES_OWNING;
  int m;
};

static struct mid g = { NULL, 0 };

int main(void)
{
  g.b = (struct leaf *)calloc(1, sizeof(struct leaf));
  if (g.b != NULL) {
    g.b->x = (int *)malloc(sizeof(int));
    if (g.b->x != NULL) {
      *g.b->x = 1;
    }
  }
  return 0;
}

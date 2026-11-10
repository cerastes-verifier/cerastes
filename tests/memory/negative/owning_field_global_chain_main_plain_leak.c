// expect: error leak @ L31
// mode: c23, c++20
// twin: tests/memory/owning_field_global_chain_main.c
// Pins: 0055 item 5 (S4): the block goes to the plain field g.b->y at the same depth: only the owning leaves' blocks are exempt at main's end
// Pre-registered from audit round 1 (record 0055 as revised, 78838ca), leak at main's closing brace (the exemption names the owning leaves only)
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
    g.b->y = (int *)malloc(sizeof(int));
    if (g.b->y != NULL) {
      *g.b->y = 1;
    }
  }
  return 0;
}  // <- here

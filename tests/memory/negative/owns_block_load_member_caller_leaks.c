// expect: error leak @ L34
// mode: c23, c++20
// twin: tests/memory/owns_block_load_member.c
// Pins: 0054 item 2: the block given through p->q is the caller's; never freed, it is a leak at the closing brace
// Pre-registered from record 0054 item 2 (given): leak at the closing brace
#include <cerastes.h>
#include <stdlib.h>

struct node {
  int *q;
  int tag;
};

void attach(struct node *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p) && cerastes_owns_block(p->q) && *p->q == 7)
{
  p->q = (int *)malloc(sizeof(int));
  if (p->q == NULL) {
    abort();
  }
  *p->q = 7;
  p->tag = 1;
}

int use_attach(void)
    CERASTES_ENSURES(r, r == 7)
{
  struct node n = {NULL, 0};
  attach(&n);
  int *q = n.q;
  int v = *q;
  return v;
}  // <- here

// expect: error postcondition @ L17
// mode: c23, c++20
// twin: tests/memory/heap_block_then_load.c
// The twin returns a value the ensures does not promise (record 0047 item 4; audit round 2 S3).
#include <cerastes.h>
#include <stdlib.h>

struct N {
  int *q;
  int v;
};

int through_block(struct N *p)
    CERASTES_REQUIRES(cerastes_owns_block(p) && cerastes_owns(*p->q))
    CERASTES_ENSURES(r, cerastes_owns_block(p) && p->q == cerastes_old(p->q) && cerastes_owns(*p->q) && r == *p->q)
{
  return 7;
}

int guarded(struct N *p)
    CERASTES_REQUIRES(cerastes_owns_block(p) && (p->q != NULL ? cerastes_owns(*p->q) : true))
    CERASTES_ENSURES(r, cerastes_owns_block(p) && p->q == cerastes_old(p->q) && (p->q != NULL ? cerastes_owns(*p->q) : true) && r == 0)
{
  return 0;
}

int use_it(void)
{
  int x = 5;
  struct N *n = (struct N *)malloc(sizeof(struct N));
  if (n == NULL) {
    return -1;
  }
  n->q = &x;
  n->v = 0;
  int r = through_block(n);
  CERASTES_CHECK(r == x);
  free(n);
  return 0;
}

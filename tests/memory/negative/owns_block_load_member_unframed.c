// expect: error permission @ L16
// expect: error permission @ L24
// mode: c23, c++20
// twin: tests/memory/owns_block_load_member.c
// Pins: 0054 item 2 (framing): p->q is read before any atom of the ensures frames it: permission at the clause (f__wf). Pre-registered from record 0054 item 2 ("When it is read": an unframed load is permission in f__wf); after the first run (question 183): also permission at the return row (L24), whose body cannot hold the unframed place either, as an unframed owns(**pp) does
#include <cerastes.h>
#include <stdlib.h>

struct node {
  int *q;
  int tag;
};

void attach(struct node *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns_block(p->q))  // <- here
{
  p->q = (int *)malloc(sizeof(int));
  if (p->q == NULL) {
    abort();
  }
  *p->q = 7;
  p->tag = 1;
}

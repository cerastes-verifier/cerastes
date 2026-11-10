// expect: error invalid-free @ L29
// expect: error invalid-free @ L29
// expect: error leak @ L35
// expect: error permission @ L35
// expect: error leak @ L36
// expect: error permission @ L36
// expect: error permission @ L36
// mode: c23, c++20
// twin: tests/memory/owning_field_loop_replace.c
// Pins: 0055 item 7 (a false alarm pinned, not a defect of the program): a loop that frees and replaces v->data through a non-owning alias, struct buf *v = p;, of a borrowed parameter is rejected today: the heap place of a struct pointer (0047 item 10) does not carry its pointee's closure, so the free of v->data is not backed by anything the loop holds and the loop gives p->data back as an unowned block; the same loop on p->data directly verifies (owning_field_loop_replace.c). The expectations are the errors the encoder reports today (the first is the invalid-free at the free of v->data); lifting them is a later session's (the deferred list)
// Pre-registered from the il-analyst's probe il/p/hp.c, observed on aux 6a27688: the expect lines are the actual outcome of the program, recorded as a pin
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

void churn(struct buf *p, int n)
    CERASTES_REQUIRES(cerastes_owns(*p) && 0 <= n && n <= 5)
    CERASTES_ENSURES(cerastes_owns(*p))
{
  struct buf *v = p;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    free(v->data);
    v->data = (int *)malloc(sizeof(int));
    if (v->data != NULL) {
      *v->data = i;
    }
    i += 1;
  }
}

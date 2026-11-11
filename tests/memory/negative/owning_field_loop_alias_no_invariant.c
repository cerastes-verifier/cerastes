// expect: error invalid-free @ L29
// expect: error invalid-free @ L29
// expect: error leak @ L35
// expect: error permission @ L35
// expect: error leak @ L36
// expect: error permission @ L36
// Record 0058's round-2 B1 reorder leaves one fewer duplicate `permission` at this line (owner question 220); the line was: expect: error permission @ L36
// mode: c23, c++20
// twin: tests/memory/owning_field_loop_alias_written.c
// Pins: 0056 item 8 (question 201; not a false alarm): a loop that frees and replaces v->data through a non-owning alias, struct buf *v = p;, of a borrowed parameter needs the written invariant CERASTES_INVARIANT(cerastes_owns(*p)), as owning_field_loop_replace.c has it (S41 (0) run 1a: with it the loop verifies, 1b: owns(*v) in its place is rejected as an invariant owning v->data, which may be *p->data; 1c: the same loop on p->data directly without the invariant fails the same way); without it the loop gives p->data back as an unowned block. The expectations are the errors the encoder reports today, kept by the owner's answer 201 (the first is the invalid-free at the free of v->data)
// Pre-registered from the il-analyst's probe il/p/hp.c, observed on aux 6a27688: the expect lines are the actual outcome of the program; M4.5d2b2c keeps them (question 201) and names the invariant the loop needs
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

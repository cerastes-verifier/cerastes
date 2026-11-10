// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0055 items 3, 5: &s of a struct local goes to an initializer, a filler and a clearer: each callee borrows owns(*p) with the closure of the field, and after each call the leaf's ownership follows s.data != NULL (the clearer says p->data == NULL, so the exit row finds nothing held)
// Pre-registered from record 0055 item 5 (named leaves: new, no base form): verifies in both modes (the callees' contracts are today's forms; the ghost update after a call that takes &s is new)
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

void buf_init(struct buf *p CERASTES_MUST_INIT)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  p->data = NULL;
  p->n = 0;
}

void buf_fill(struct buf *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  free(p->data);
  p->data = (int *)malloc(sizeof(int));
  if (p->data != NULL) {
    *p->data = 7;
  }
  p->n = 1;
}

void buf_clear(struct buf *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p) && p->data == NULL)
{
  free(p->data);
  p->data = NULL;
}

int use(void)
{
  struct buf s;
  buf_init(&s);
  buf_fill(&s);
  buf_clear(&s);
  return 0;
}

// expect: verified
// expect: native-ok
// mode: c23, c++20
// verify-options: --strict-init
// What a marked member may receive: malloc's result, a null pointer, &x of an uninit local, another marked pointer; and it may be passed to a ref_to_uninit parameter.
// tag::source[]
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING CERASTES_REF_TO_UNINIT;
  int n;
};

struct cursor {
  int *target CERASTES_REF_TO_UNINIT;
  int n;
};

void refill(struct buf *b)
    CERASTES_REQUIRES(cerastes_owns(*b))
    CERASTES_ENSURES(cerastes_owns(*b))
{
  free(b->data);
  b->data = (int *)malloc(sizeof(int));
}

void clear(struct cursor *c)
    CERASTES_REQUIRES(cerastes_owns(*c))
    CERASTES_ENSURES(cerastes_owns(*c))
{
  c->target = NULL;
}

void set(int *x CERASTES_REF_TO_UNINIT)
    CERASTES_REQUIRES(cerastes_owns(*x))
    CERASTES_ENSURES(cerastes_owns(*x))
{
  *x = 1;
}

void use(struct cursor *c)
    CERASTES_REQUIRES(cerastes_owns(*c) && c->target != NULL && cerastes_owns(*c->target))
    CERASTES_ENSURES(cerastes_owns(*c) && c->target != NULL && cerastes_owns(*c->target))
{
  set(c->target);
}

int main(void)
{
  int x CERASTES_UNINIT;
  struct cursor a = { NULL, 0 };
  a.target = &x;
  *a.target = 1;
  struct cursor b = { NULL, 0 };
  int *t CERASTES_REF_TO_UNINIT = b.target;
  a.target = t;
  a.target = b.target;
  return a.n + b.n;
}
// end::source[]

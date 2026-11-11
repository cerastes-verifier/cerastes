// expect: verified
// expect: native-ok
// mode: c23, c++20
// verify-options: --strict-owner
// Every allowed position of an owning result and of a release under --strict-owner.
// tag::source[]
#include <cerastes.h>
#include <stdlib.h>

struct account {
  int balance;
};

CERASTES_OWNING struct account *create(void)
{
  struct account *a CERASTES_OWNING = (struct account *)malloc(sizeof(struct account));
  if (a != NULL) {
    a->balance = 0;
  }
  return a;
}

CERASTES_OWNING struct account *create_again(void)
{
  return create();
}

void destroy(struct account *a)
    CERASTES_REQUIRES(a == NULL || cerastes_owns_block(a))
{
  free(a);
}

int use(void)
    CERASTES_ENSURES(r, r == 5 || r == -1)
{
  struct account *a CERASTES_OWNING = create();
  if (a == NULL) {
    return -1;
  }
  a->balance = 5;
  int b = a->balance;
  destroy(a);
  destroy(create_again());
  return b;
}
// end::source[]

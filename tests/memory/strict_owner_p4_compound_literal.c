// expect: verified
// expect: native-ok
// c-only: the address of a compound literal is unsupported_expression in C++ (record 0044 item 7), with or without the option
// verify-options: --strict-owner
// Pins: 0058 item 3 (p4): an owning field's leaf in a compound literal ({ make(), 0 }) is an owner position; c23 only: the address of a compound literal is unsupported_expression in C++ (record 0044 item 7), with or without the option
// Pre-registered from record 0058 item 3: verifies in c23 under --strict-owner
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

CERASTES_OWNING int *make(void)
{
  CERASTES_OWNING int *a = (int *)malloc(sizeof(int));
  if (a != NULL) {
    *a = 0;
  }
  return a;
}

int f(void)
    CERASTES_ENSURES(r, r == 0)
{
  struct buf *c = &(struct buf){ make(), 0 };
  free(c->data);
  return 0;
}

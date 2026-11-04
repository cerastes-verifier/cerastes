// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0047 item 5 (P4222's marker on a function, target r): the result's block has no value in its cells (the ensures atom's flag rows are dropped), so the callee may return malloc's block as it is and the caller writes before it reads
// Pre-registered from record 0047 item 5: verifies (the marker needs owns_block(r) in an ensures, which is there)
#include <cerastes.h>
#include <stdlib.h>

struct account {
  int balance;
};

CERASTES_REF_TO_UNINIT struct account *create_raw(void)
    CERASTES_ENSURES(r, cerastes_owns_block(r))
{
  struct account *a = (struct account *)malloc(sizeof(struct account));
  if (a == NULL) {
    abort();
  }
  return a;
}

int use_raw(void)
    CERASTES_ENSURES(r, r == 5)
{
  struct account *a = create_raw();
  a->balance = 5;
  int b = a->balance;
  free(a);
  return b;
}

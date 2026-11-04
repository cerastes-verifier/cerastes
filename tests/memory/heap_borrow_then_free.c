// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0047 item 4: the same atom in a requires and an ensures on a formal borrows the block and returns it whole, so the caller can still free it
// Pre-registered from record 0047 item 4: verifies
#include <cerastes.h>
#include <stdlib.h>

struct account {
  int balance;
};

void set_balance(struct account *a, int n)
    CERASTES_REQUIRES(cerastes_owns_block(a))
    CERASTES_ENSURES(cerastes_owns_block(a) && a->balance == n)
{
  a->balance = n;
}

int borrow_then_free(void)
    CERASTES_ENSURES(r, r == 5 || r == -1)
{
  struct account *a = (struct account *)malloc(sizeof(struct account));
  if (a == NULL) {
    return -1;
  }
  a->balance = 0;
  set_balance(a, 5);
  int b = a->balance;
  free(a);
  return b;
}

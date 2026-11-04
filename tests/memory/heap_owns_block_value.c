// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0047 item 4 (0028 item 6, VeriFast's account_balance(result, 0) &*& malloc_block_account(result)): owns_block on the result in an ensures frames the read r->balance of the same clause, and the caller reads the value
// Pre-registered from record 0047 item 4 (the atom frames reads of its cells): verifies
#include <cerastes.h>
#include <stdlib.h>

struct account {
  int balance;
};

struct account *create_account(void)
    CERASTES_ENSURES(r, cerastes_owns_block(r) && r->balance == 0)
{
  struct account *a = (struct account *)malloc(sizeof(struct account));
  if (a == NULL) {
    abort();
  }
  a->balance = 0;
  return a;
}

int use_account(void)
    CERASTES_ENSURES(r, r == 0)
{
  struct account *a = create_account();
  int b = a->balance;
  CERASTES_CHECK(b == 0);
  free(a);
  return b;
}

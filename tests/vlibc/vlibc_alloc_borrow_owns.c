// expect: verified
// expect: native-ok
// mode: c23
// c-only: vlibc_malloc creates no C++ objects (record 0050 item 7)
// Pins: 0050 item 5 (0047 item 4): a vlibc block lent to a callee whose contract names its cells (owns(*a) in both clauses) comes back whole, and vlibc_free ends it
// Pre-registered from record 0050 item 5: verifies (only owns_block and owning need the platform's value)
#include <vlibc/stdlib.h>
#include <cerastes.h>

struct account {
  int balance;
};

void set_balance(struct account *a, int n)
    CERASTES_REQUIRES(cerastes_owns(*a))
    CERASTES_ENSURES(cerastes_owns(*a) && a->balance == n)
{
  a->balance = n;
}

int borrow_then_free(void)
    CERASTES_ENSURES(r, r == 5 || r == -1)
{
  struct account *a = (struct account *)vlibc_calloc(1, sizeof(struct account));
  if (a == NULL) {
    return -1;
  }
  set_balance(a, 5);
  int b = a->balance;
  vlibc_free(a);
  return b;
}

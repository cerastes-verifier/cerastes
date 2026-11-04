// expect: verified
// expect: native-ok
// Pins: 0047 item 4: cerastes::owns_block(p) is the C++ spelling of cerastes_owns_block(p)
// Pre-registered from record 0047 item 4: verifies
#include <cerastes.h>
#include <stdlib.h>

struct account {
  int balance;
};

account *create_account()
    CERASTES_ENSURES(r, cerastes::owns_block(r) && r->balance == 0)
{
  account *a = static_cast<account *>(malloc(sizeof(account)));
  if (a == nullptr) {
    abort();
  }
  a->balance = 0;
  return a;
}

void dispose(account *a)
    CERASTES_REQUIRES(cerastes::owns_block(a))
{
  free(a);
}

int use_account()
    CERASTES_ENSURES(r, r == 0)
{
  account *a = create_account();
  int b = a->balance;
  dispose(a);
  return b;
}

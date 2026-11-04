// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0047 item 4: owns_block in a requires alone takes the whole block (no ensures): the callee frees it, the caller no longer holds it
// Pre-registered from record 0047 item 4: verifies
#include <cerastes.h>
#include <stdlib.h>

struct account {
  int balance;
};

void dispose(struct account *a)
    CERASTES_REQUIRES(cerastes_owns_block(a))
{
  free(a);
}

int make_and_dispose(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  struct account *a = (struct account *)malloc(sizeof(struct account));
  if (a == NULL) {
    return -1;
  }
  a->balance = 5;
  dispose(a);
  return 0;
}

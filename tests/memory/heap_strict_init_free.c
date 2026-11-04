// expect: verified
// expect: native-ok
// mode: c23, c++20
// verify-options: --strict-init
// Pins: 0047 item 8 (amends 0042 item 8): under --strict-init free's ref_to_uninit parameter is exempt from the rule that an argument to a marked parameter is a marked local's address or the caller's own marked parameter, and calloc (unmarked) is accepted: only calls of ref_to_uninit-result functions (malloc) are rejected
// Pre-registered from record 0047 item 8: verifies under --strict-init; re-registered after audit round 1 (record 0047 item 8): the positive allocates with calloc: malloc is rejected under --strict-init wherever it stands
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

int make_zeroed(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  struct account *a = (struct account *)calloc(1, sizeof(struct account));
  if (a == NULL) {
    return -1;
  }
  int b = a->balance;
  dispose(a);
  return b;
}

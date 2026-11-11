// expect: verified
// expect: native-ok
// mode: c23, c++20
// verify-options: --strict-init
// twin-exempt: a documented blind spot of a static opt-in (record 0058 item 4, audit S4): nothing in it can fail; its must-fail form is the rejected direct store reject_strict_marked_field_address.c
// Pins: 0058 item 4 (audit S4, "accepted although wrong"): a store to a marked field through an alias, int **pp = &s.q; *pp = &x;, reaches no marked target: --strict-init accepts it (the same blind spot as an address-taken marked local, 0052 item 4); the direct store of the same address is rejected by reject_strict_marked_field_address.c
// Pre-registered from record 0058 item 4: verifies in both modes under --strict-init
#include <cerastes.h>
#include <stdlib.h>

struct pt {
  int *q CERASTES_REF_TO_UNINIT;
  int n;
};

int f(void)
    CERASTES_ENSURES(r, r == 0)
{
  int x = 1;
  struct pt s = { NULL, 0 };
  int **pp = &s.q;
  *pp = &x;
  return s.n;
}

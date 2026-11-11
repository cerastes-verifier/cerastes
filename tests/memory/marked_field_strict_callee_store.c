// expect: verified
// expect: native-ok
// mode: c23, c++20
// verify-options: --strict-init
// twin-exempt: a documented blind spot of a static opt-in (record 0058 item 4, audit S4): nothing in it can fail; its must-fail form is the rejected direct store reject_strict_marked_field_address.c
// Pins: 0058 item 4 (audit S4, "accepted although wrong"): a callee given the address of a marked field writes through it, set(&s.q): the argument is a pointer to the field, not a store to it, so --strict-init accepts it
// Pre-registered from record 0058 item 4: verifies in both modes under --strict-init
#include <cerastes.h>
#include <stdlib.h>

struct pt {
  int *q CERASTES_REF_TO_UNINIT;
  int n;
};

static int gv = 1;

void set(int **pp)
    CERASTES_REQUIRES(cerastes_owns(*pp))
    CERASTES_ENSURES(cerastes_owns(*pp))
{
  *pp = &gv;
}

int f(void)
    CERASTES_ENSURES(r, r == 0)
{
  struct pt s = { NULL, 0 };
  set(&s.q);
  return s.n;
}

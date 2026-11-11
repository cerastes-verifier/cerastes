// expect: rejected cerastes-uninit_ref_initialized @ L20
// mode: c23, c++20
// verify-options: --strict-init
// twin: tests/memory/marked_field_strict_store.c
// Pins: 0058 item 4 (under --strict-init): p->q = &x of an initialized object into a marked field is rejected as the same store into a marked local is (0052 item 4)
// Pre-registered from record 0058 item 4: rejected uninit_ref_initialized at the store (observed on the Part F build: build/m4.5d2b2d/emul3/sf2.c)
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
  s.q = &x;  // <- here
  return s.n;
}

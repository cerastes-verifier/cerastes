// expect: rejected cerastes-uninit_ref_initialized @ L19
// mode: c23, c++20
// verify-options: --strict-init
// twin: tests/memory/marked_field_strict_init_list.c
// Pins: 0058 item 4 (under --strict-init): the address of an initialized object as a marked leaf's initializer is rejected
// Pre-registered from record 0058 item 4: rejected uninit_ref_initialized at the initializer (observed: build/m4.5d2b2d/emul3/sf3.c)
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
  struct pt s = { &x, 0 };  // <- here
  return s.n;
}

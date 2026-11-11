// expect: rejected cerastes-uninit_ref_initialized @ L20
// mode: c23, c++20
// verify-options: --strict-init
// twin: tests/memory/marked_field_strict_global.c
// Pins: 0058 item 4 (audit S4: static-local struct initializers beside the global ones): a static local's initializer with the address of an object of static storage is rejected
// Pre-registered from record 0058 item 4: rejected uninit_ref_initialized at the initializer
#include <cerastes.h>
#include <stdlib.h>

struct pt {
  int *q CERASTES_REF_TO_UNINIT;
  int n;
};

static int v = 1;

int local(void)
    CERASTES_ENSURES(r, r == 0)
{
  static struct pt sl = { &v, 0 };  // <- here
  return sl.n;
}

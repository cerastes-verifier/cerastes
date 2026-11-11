// expect: rejected cerastes-uninit_ref_initialized @ L16
// mode: c23, c++20
// verify-options: --strict-init
// twin: tests/memory/marked_field_strict_global.c
// Pins: 0058 item 4 (under --strict-init): a global's static initializer that takes the address of an object of static storage (it always holds a value) is rejected
// Pre-registered from record 0058 item 4: rejected uninit_ref_initialized at the initializer (observed: build/m4.5d2b2d/emul3/sf5.c)
#include <cerastes.h>
#include <stdlib.h>

struct pt {
  int *q CERASTES_REF_TO_UNINIT;
  int n;
};

static int v = 1;
struct pt h = { &v, 0 };  // <- here

int main(void)
{
  return 0;
}

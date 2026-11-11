// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0058 item 4 (flip of reject_ref_to_uninit_field.c): ref_to_uninit on a pointer field that is not owning is accepted and, in the default mode, documents intent and changes no Viper (as 0052 item 4 for a local)
// Pre-registered from record 0058 item 4: verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

struct S {
  int *q CERASTES_REF_TO_UNINIT;
  int k;
};

int get(void)
    CERASTES_ENSURES(r, r == 5)
{
  struct S s = { NULL, 5 };
  return s.k;
}

// expect: verified
// expect: native-ok
// mode: c23, c++20
// verify-options: --strict-init
// Pins: 0058 item 4 (under --strict-init): per-leaf initializers: a null pointer for a marked non-owning field and the result of malloc (a ref_to_uninit call) for an owning marked one are accepted
// Pre-registered from record 0058 item 4: verifies in both modes under --strict-init (observed: build/m4.5d2b2d/emul3/sf4.c)
#include <cerastes.h>
#include <stdlib.h>

struct pt {
  int *q CERASTES_REF_TO_UNINIT;
  int n;
};

struct buf {
  int *data CERASTES_OWNING CERASTES_REF_TO_UNINIT;
  int n;
};

int use(void)
    CERASTES_ENSURES(r, r == 0)
{
  struct pt a = { NULL, 0 };
  struct buf b = { (int *)malloc(sizeof(int)), 0 };
  free(b.data);
  return a.n;
}

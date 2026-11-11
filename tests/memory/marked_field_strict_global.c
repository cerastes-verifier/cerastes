// expect: verified
// expect: native-ok
// mode: c23, c++20
// verify-options: --strict-init
// Pins: 0058 item 4 (under --strict-init): a global's and a static local's static initializer gives a marked field a null pointer (accepted)
// Pre-registered from record 0058 item 4: verifies in both modes under --strict-init (observed: build/m4.5d2b2d/emul3/sf6.c)
#include <cerastes.h>
#include <stdlib.h>

struct pt {
  int *q CERASTES_REF_TO_UNINIT;
  int n;
};

struct pt g = { NULL, 1 };

int local(void)
    CERASTES_ENSURES(r, r == 3)
{
  static struct pt sl = { NULL, 2 };
  sl.n = 3;
  return sl.n;
}

int main(void)
{
  return 0;
}

// expect: verified
// expect: native-ok
// c-only: the address of a compound literal is unsupported_expression in C++ (record 0044 item 7); a compound literal as a value is a GNU extension there
// verify-options: --strict-init
// Pins: 0058 item 4 (under --strict-init): a compound literal's marked leaf takes a null pointer (c23 only)
// Pre-registered from record 0058 item 4: verifies in c23 under --strict-init
#include <cerastes.h>
#include <stdlib.h>

struct pt {
  int *q CERASTES_REF_TO_UNINIT;
  int n;
};

int use(void)
    CERASTES_ENSURES(r, r == 0)
{
  struct pt s = (struct pt){ NULL, 0 };
  return s.n;
}

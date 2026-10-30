// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0040 item 3 (audit N2): owning on a parameter of a typedef'd function type: EDG merges the typedef's parameter attribute into the function's, so the declaration and the definition agree
#include <cerastes.h>
#include <stddef.h>

typedef void fn_t(int *p CERASTES_OWNING);
fn_t f;

void f(int *p CERASTES_OWNING)
{
  if (p) {
    *p = 1;
  }
}

int use_f(void)
    CERASTES_ENSURES(r, r == 3)
{
  int x = 0;
  f(&x);
  f(NULL);
  x = 3;
  return x;
}

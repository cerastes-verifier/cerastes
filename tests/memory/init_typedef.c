// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0042 item 4 (audit S4): must_init on a parameter of a typedef'd function type: EDG merges the typedef's attribute into the routine's, so the declaration and the definition agree
#include <cerastes.h>

typedef void fn_t(int *p CERASTES_MUST_INIT);
fn_t init;

void init(int *p CERASTES_MUST_INIT)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p) && *p == 0)
{
  *p = 0;
}

int use_init(void)
    CERASTES_ENSURES(r, r == 0)
{
  int x;
  init(&x);
  return x;
}

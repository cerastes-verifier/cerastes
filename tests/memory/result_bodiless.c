// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0052 item 2a (question 158; vlibc's shape): a bodiless contract with a partnerless atom on a fresh result (r == NULL || owns(*r)): accepted, since Viper's call picks a place the caller does not hold; the caller tests the result before writing; the lint pins the amount's row in the emitted Viper
// Pre-registered from record 0052 item 2a: verifies in both modes
#include <cerastes.h>
#include <stddef.h>

int *fresh(void)
    CERASTES_ENSURES(r, r == NULL || cerastes_owns(*r));

int use_fresh(void)
    CERASTES_ENSURES(r, r == 0 || r == 2)
{
  int *p = fresh();
  int w = 0;
  if (p != NULL) {
    *p = 2;
    w = *p;
  }
  return w;
}

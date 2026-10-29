// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0037 items 5, 6, 8: p == NULL || owns(*p): the atom is guarded, the amount of a false guard is write / (N + 1), the call with NULL and with &x
#include <cerastes.h>
#include <stddef.h>

int get(int *p)
    CERASTES_REQUIRES(p == NULL || cerastes_owns(*p))
    CERASTES_ENSURES(r, (p == NULL || cerastes_owns(*p)) && (p == NULL ? r == 0 : (r == *p && *p == cerastes_old(*p))))
{
  if (p == NULL) {
    return 0;
  }
  return *p;
}

int use_get(void)
    CERASTES_ENSURES(r, r == 5)
{
  int x = 5;
  int a = get(&x);
  int b = get(NULL);
  return a + b;
}

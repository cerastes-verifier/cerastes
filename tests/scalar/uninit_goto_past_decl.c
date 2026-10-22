// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0019 item 6: a forward goto may jump past a declaration without an initializer
#include <cerastes.h>

int skip_decl(int a)
    CERASTES_ENSURES(r, r == (a > 5 ? 1 : a + 1))
{
  int total = 0;
  if (a > 5) {
    goto done;
  }
  int x;
  x = a;
  total = x;
done:
  total += 1;
  return total;
}

int assign_at_label(int a)
    CERASTES_ENSURES(r, r == (a > 5 ? 7 : 10))
{
  int y = 0;
  if (a > 5) {
    goto done;
  }
  int x;
  x = 3;
  y = x;
done:
  x = 7;
  return y + x;
}

// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0059 item 4 (0037 item 8's case splits over a quantified chunk): swap(&a[i], &a[j]) with symbolic indices that differ
// Pre-registered from record 0059 item 4: verifies
#include <cerastes.h>

void swap(int *a, int *b)
    CERASTES_REQUIRES(cerastes_owns(*a) && cerastes_owns(*b))
    CERASTES_ENSURES(cerastes_owns(*a) && cerastes_owns(*b) && *a == cerastes_old(*b) && *b == cerastes_old(*a))
{
  int t = *a;
  *a = *b;
  *b = t;
}

int swap_at(int i, int j)
    CERASTES_REQUIRES(0 <= i && i < 3 && 0 <= j && j < 3 && i != j)
{
  int a[3] = {1, 2, 3};
  int x = a[i];
  int y = a[j];
  swap(&a[i], &a[j]);
  CERASTES_CHECK(a[i] == y);
  CERASTES_CHECK(a[j] == x);
  return x - y;
}

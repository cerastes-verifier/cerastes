// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0063 items 7 and 9 (0041 item 12): a static array local is an implicit owns atom in the contracts of its function and of every function that reaches it (twice, main); its entry value is unknown, so the function guards the counter it reads
// Pre-registered from record 0063 item 9 (Part B), before any run: verifies
#include <cerastes.h>

int next(int i)
    CERASTES_REQUIRES(0 <= i && i < 3)
    CERASTES_ENSURES(r, 1 <= r && r <= 100)
{
  static int c[3] = {0};
  int v = c[i];
  if (v < 0 || v > 99) {
    v = 0;
  }
  c[i] = v + 1;
  return c[i];
}

int twice(void)
    CERASTES_ENSURES(r, 2 <= r && r <= 200)
{
  int a = next(0);
  int b = next(1);
  return a + b;
}

int main(void)
{
  int a = next(2);
  CERASTES_CHECK(1 <= a && a <= 100);
  (void)a;
  int t1 = twice();
  int t2 = twice();
  return t1 - t2;
}

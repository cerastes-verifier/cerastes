// expect: golden
// expect: native-ok
// mode: c++20
// M3 probe P6: parameter references in a contract, in the body and in statement annotations.
#include <cerastes.h>

int f(int a, int b) CERASTES_REQUIRES(a >= 0 && b > a) CERASTES_ENSURES(r, r >= a)
{
  int s = a;
  CERASTES_CHECK(s == a);
  a = a + 1;
  CERASTES_CHECK(a > s && b > s);
  CERASTES_INVARIANT(s <= b) while (s < b) { s = s + 1; }
  return s + b;
}

// expect: verified
// expect: native-ok
// mode: c23, c++20
// assert from <assert.h> is checked like CERASTES_CHECK, with the meaning it has at run time.
// tag::source[]
#include <assert.h>
#include <cerastes.h>

int average(int a, int b)
    CERASTES_REQUIRES(0 <= a && a <= 100 && 0 <= b && b <= 100)
    CERASTES_ENSURES(r, r == (a + b) / 2)
{
  int sum = a + b;
  assert(sum <= 200);
  return sum / 2;
}
// end::source[]

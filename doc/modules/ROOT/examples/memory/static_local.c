// expect: verified
// expect: native-ok
// mode: c23, c++20
// A static local variable: its function and every function that calls it hold it, and its value is unknown each time
// the function is entered.
// tag::source[]
#include <cerastes.h>
#include <limits.h>

int next_id(void)
    CERASTES_ENSURES(r, r > 0)
{
  static int last = 0;
  if (last < 0 || last == INT_MAX)
    last = 0;
  last += 1;
  return last;
}

int main(void)
{
  int a = next_id();
  int b = next_id();
  CERASTES_CHECK(a > 0 && b > 0);
  (void)a;
  (void)b;
  return 0;
}
// end::source[]

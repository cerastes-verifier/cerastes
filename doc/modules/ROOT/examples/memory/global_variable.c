// expect: verified
// expect: native-ok
// mode: c23, c++20
// A global variable: main starts with its initial value, a function reaches it only through its contract, and a
// function whose contract does not name it cannot change it.
// tag::source[]
#include <cerastes.h>

static int count = 0;

void record(void)
    CERASTES_REQUIRES(cerastes_owns(count) && count < 1000)
    CERASTES_ENSURES(cerastes_owns(count) && count == cerastes_old(count) + 1)
{
  count += 1;
}

int total(void)
    CERASTES_REQUIRES(cerastes_reads(count))
    CERASTES_ENSURES(r, cerastes_reads(count) && r == count)
{
  return count;
}

int twice(int a)
    CERASTES_REQUIRES(a >= 0 && a <= 1000)
    CERASTES_ENSURES(r, r == 2 * a)
{
  return 2 * a;
}

int main(void)
{
  record();
  record();
  int t = twice(total());
  CERASTES_CHECK(count == 2 && t == 4);
  (void)t;
  return 0;
}
// end::source[]

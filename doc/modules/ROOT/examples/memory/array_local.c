// expect: verified
// expect: native-ok
// mode: c23, c++20
// tag::source[]
#include <cerastes.h>

int score(int i)
    CERASTES_REQUIRES(0 <= i && i < 4)
    CERASTES_ENSURES(r, 0 <= r && r <= 30)
{
  int table[4] = {10, 20};
  table[3] = 30;
  CERASTES_CHECK(table[2] == 0);
  return table[i];
}
// end::source[]

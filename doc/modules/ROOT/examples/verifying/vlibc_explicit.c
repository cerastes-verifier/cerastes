// expect: verified
// expect: native-ok
// mode: c23, c++20
// The explicit include reads the same contract in every build, not only when cerastes verifies the file.
// tag::source[]
#include <vlibc/stdlib.h>
#include <cerastes.h>

int non_negative(int x)
    CERASTES_REQUIRES(-100 <= x && x <= 100)
    CERASTES_ENSURES(r, r >= 0 && r == x)
{
  if (x < 0) {
    abort();
  }
  return x;
}
// end::source[]

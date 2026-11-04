// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0011 items 5-6 and 0046 item 8 (the positive of the redirections): a bodiless declaration with a contract in a user file is trusted when nothing redirects its calls
// g is defined in another unit (the single-unit assumption); its contract is what the caller gets.
#include <cerastes.h>

int g(int x)
    CERASTES_REQUIRES(0 <= x && x <= 100)
    CERASTES_ENSURES(r, r == x + 1);

int use(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
    CERASTES_ENSURES(r, r == x + 1)
{
  return g(x);
}

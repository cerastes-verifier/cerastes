// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0041 item 2: the accepted types of a mutable global: an enumeration, an unsigned char, a long long and a bool, each with its initializer's value and range
#include <cerastes.h>

enum color { RED, GREEN = 5 };

static enum color col = GREEN;
static unsigned char uc = 200;
static long long ll = 1099511627776LL;
static bool flag = true;

int main(void)
{
  CERASTES_CHECK(col == GREEN && uc == 200 && ll == 1099511627776LL && flag);
  uc += 55;
  ll += 1;
  CERASTES_CHECK(uc == 255 && ll == 1099511627777LL);
  return (flag && col == GREEN) ? 0 : 1;
}

// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0012 item 9: main returns 0 when it falls off its end
#include <cerastes.h>

int main(void)
{
  int x = 1;
  if (x > 5) {
    return 3;
  }
  CERASTES_CHECK(x == 1);
}

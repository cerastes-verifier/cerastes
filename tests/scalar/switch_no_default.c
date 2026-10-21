// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0014 item 6: without a default, a value that matches no case continues after the switch
#include <cerastes.h>

int name_of(int k)
    CERASTES_ENSURES(r, (k == 0 && r == 10) || (k == 1 && r == 11) || (k != 0 && k != 1 && r == -1))
{
  switch (k) {
  case 0:
    return 10;
  case 1:
    return 11;
  }
  return -1;
}

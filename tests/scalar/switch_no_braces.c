// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0014 item 6: a switch whose body is one labelled statement without braces
#include <cerastes.h>

int one(int x)
    CERASTES_ENSURES(r, (x == 1 && r == 1) || (x != 1 && r == 0))
{
  switch (x)
  case 1:
    return 1;
  return 0;
}

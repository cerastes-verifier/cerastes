// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0014 item 6: the scrutinee of a switch with no cases is still evaluated, with its obligations
#include <cerastes.h>

int ignore(int x, int y)
    CERASTES_REQUIRES(y > 0)
    CERASTES_ENSURES(r, r == x)
{
  switch (x / y) {
  }
  return x;
}

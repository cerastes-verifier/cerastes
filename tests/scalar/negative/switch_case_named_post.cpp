// expect: error postcondition @ L17
// twin: tests/scalar/switch_case_named.cpp
// Pins: 0017 item 6: the exact ensures notices a wrong result in the case N + 1
#include <cerastes.h>

const int N = 3;

int classify(int x)
    CERASTES_ENSURES(r, r == (x == 3 ? 1 : x == 4 ? 2 : x == 10 ? 3 : x == 20 ? 4 : 0))
{
  const int k = 10;
  constexpr int n = 20;
  switch (x) {
  case N:
    return 1;
  case N + 1:
    return 5;
  case k:
    return 3;
  case n:
    return 4;
  default:
    return 0;
  }
}

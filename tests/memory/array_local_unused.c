// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0012 item 12, retired by M4.5e1a (record 0059; plan 3.8 Flip A): the program of the former scalar/negative/reject_array.c (an unused array local, rejected until arrays were encoded) is accepted now
#include <cerastes.h>

int first(int a)
    CERASTES_ENSURES(r, r == a)
{
  [[maybe_unused]] int values[3] = {1, 2, 3};
  return a;
}

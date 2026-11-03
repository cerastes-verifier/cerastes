// expect: error postcondition @ L16
// twin: tests/memory/struct_init_literal_cpp.cpp
// Pins: 0044 item 7: s.x is n, not 2: the postcondition r == 2 fails at the return
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int compound_init(int n)
    CERASTES_REQUIRES(n > 0 && n < 10)
    CERASTES_ENSURES(r, r == 2)
{
  S s = (S){n, 2};
  return s.x;  // <- here
}

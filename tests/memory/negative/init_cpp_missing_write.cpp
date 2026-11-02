// expect: error postcondition @ L14
// mode: c++20
// twin: tests/memory/init_cpp.cpp
// Pins: 0042 item 5: the C++ spelling of the missing-write twin
#include <cerastes.h>

void init(int *p CERASTES_MUST_INIT, bool c)
    CERASTES_REQUIRES(cerastes::owns(*p))
    CERASTES_ENSURES(cerastes::owns(*p))
{
  if (c) {
    *p = 0;
  }
}  // <- here

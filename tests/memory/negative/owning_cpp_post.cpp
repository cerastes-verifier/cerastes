// expect: error postcondition @ L15
// mode: c++20
// twin: tests/memory/owning_cpp.cpp
// Pins: 0040 item 3: the body adds 1, the contract says 2
#include <cerastes.h>
#include <stddef.h>

void bump(CERASTES_OWNING int *p)
    CERASTES_REQUIRES(p == nullptr || *p < 100)
    CERASTES_ENSURES(p == nullptr || *p == cerastes::old(*p) + 2)
{
  if (p != nullptr) {
    *p += 1;
  }
}

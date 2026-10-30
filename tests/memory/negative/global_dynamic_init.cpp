// expect: rejected cerastes-unsupported_declaration @ L14
// mode: c++20
// twin: tests/memory/global_linkage_cpp.cpp
// Pins: 0041 item 2: in C++ a global whose initializer is not a constant is initialized dynamically (before main, in an order the verifier does not model): rejected
#include <cerastes.h>

int twice(int x)
    CERASTES_REQUIRES(0 <= x && x < 1000)
    CERASTES_ENSURES(r, r == 2 * x)
{
  return 2 * x;
}

static int d = twice(2);

int main()
{
  return 0;
}

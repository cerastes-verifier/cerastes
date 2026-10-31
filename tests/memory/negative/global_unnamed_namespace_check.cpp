// expect: error check @ L13
// mode: c++20
// twin: tests/memory/global_unnamed_namespace.cpp
// Pins: 0041 item 3: g is 5 at program start (internal linkage): g == 6 fails
#include <cerastes.h>

namespace {
int g = 5;
}

int main()
{
  CERASTES_CHECK(g == 6);
  return 0;
}

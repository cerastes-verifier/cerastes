// expect: error check @ L12
// mode: c++20
// twin: tests/memory/global_unnamed_namespace.cpp
// Pins: 0041 item 3 (audit B1): the address of g is the initial value of the external-linkage global gp, whose initialization another unit may precede: g itself is unknown in main
#include <cerastes.h>

static int g = 0;
int *gp = &g;

int main()
{
  CERASTES_CHECK(g == 0);
  return 0;
}

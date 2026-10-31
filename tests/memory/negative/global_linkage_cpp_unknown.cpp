// expect: error check @ L11
// mode: c++20
// twin: tests/memory/global_linkage_cpp.cpp
// Pins: 0041 item 3 (question 84): the initial value of an external-linkage global in C++ is unknown: g == 5 is not a fact in main
#include <cerastes.h>

int g = 5;

int main()
{
  CERASTES_CHECK(g == 5);
  return 0;
}

// expect: error conversion @ L7
// mode: c++20
// twin: tests/memory/global_unknown_init.cpp
// Pins: 0041 item 3 and 0017 item 4: the value of an external-linkage global in C++ is unknown, but its initializer is still checked in every prologue: (signed char)300 is a conversion error at the global
#include <cerastes.h>

signed char g = (signed char)300;

int main()
{
  return g == 0;
}

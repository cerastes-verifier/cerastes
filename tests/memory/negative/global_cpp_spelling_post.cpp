// expect: error postcondition @ L14
// mode: c++20
// twin: tests/memory/global_cpp_spelling.cpp
// Pins: 0041 item 7: cerastes::old(g) is g's entry value: g == old(g) fails after g += 1
#include <cerastes.h>

static int g = 3;

void bump()
    CERASTES_REQUIRES(cerastes::owns(g) && g < 100)
    CERASTES_ENSURES(cerastes::owns(g) && g == cerastes::old(g))
{
  g += 1;
}

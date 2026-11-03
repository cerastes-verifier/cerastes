// expect: error postcondition @ L17
// mode: c++20
// twin: tests/memory/struct_cpp_spelling.cpp
// Pins: 0043 item 9: the C++ ensures claims the member grew by 2
#include <cerastes.h>

struct S {
  int x;
  int y;
};

void bump(S* p)
    CERASTES_REQUIRES(cerastes::owns(*p) && p->x < 100)
    CERASTES_ENSURES(cerastes::owns(*p) && p->x == cerastes::old(p->x) + 2 && p->y == cerastes::old(p->y))
{
  p->x += 1;
}  // <- here

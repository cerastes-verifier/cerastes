// expect: error postcondition @ L18
// mode: c++20
// twin: tests/memory/struct_array_cpp.cpp
// Pins: 0063 item 7: the C++ ensures claims the element grew by 2: postcondition at the closing brace
// Pre-registered from record 0063 item 7 (Part C), before any run: the one error
#include <cerastes.h>

struct S {
  int a[4];
  int n;
};

void bump(S* p, int i)
    CERASTES_REQUIRES(cerastes::owns(*p) && 0 <= i && i < 4 && p->a[i] < 100)
    CERASTES_ENSURES(cerastes::owns(*p) && p->a[i] == cerastes::old(p->a[i]) + 2)
{
  p->a[i] += 1;
}  // <- here

// expect: verified
// expect: native-ok
// Pins: 0044 item 8 (0041 item 3, question 84): a struct global with external linkage in C++ has an unknown initialized value in main: main may write a member and then know it
#include <cerastes.h>

struct S {
  int x;
  int y;
};

S g = {1, 2};

int main()
{
  g.x = 6;
  CERASTES_CHECK(g.x == 6);
  return g.x < g.y;
}

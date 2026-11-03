// expect: verified
// expect: native-ok
// Pins: 0044 item 8 (0041 item 5, audit N4): static S g{}; static T t = {&g}; both internal: the escape rule does not apply, g is zero in main
#include <cerastes.h>

struct S {
  int x;
  int y;
};

struct T {
  S *p;
};

static S g{};
static T t = {&g};

int main()
{
  CERASTES_CHECK(g.x == 0 && g.y == 0);
  return (t.p == &g ? 1 : 0) + g.x;
}

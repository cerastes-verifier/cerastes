// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0063 item 14: a typedef of an array type used at file scope (a static global array declared through it) and a block-scope typedef that a local array is declared through (used, so the unused-typedef rule of question 302 does not apply); the values are the written ones. The member declared through a typedef is Part C's (struct_array_typedef_member.c)
// Pre-registered from record 0063 item 14 (Part D), before any run: verifies
#include <cerastes.h>

typedef int T3[3];

static T3 g = {1, 2, 3};

int main(void)
{
  typedef int T2[2];
  T2 c = {4, 5};
  CERASTES_CHECK(g[1] == 2);
  CERASTES_CHECK(c[1] == 5);
  return g[0] + c[0] - 5;
}

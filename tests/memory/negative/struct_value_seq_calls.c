// expect: error unsequenced @ L30
// mode: c23, c++20
// twin: tests/memory/struct_value_result_use.c
// Pins: 0044 item 5 (0021, 0039 item 4): h2(mk(), mk()) with mk writing a global: two struct-valued calls are two fixed points that both access g, indeterminately sequenced: unsequenced at the second call, as for scalar results
#include <cerastes.h>

struct S {
  int x;
  int y;
};

static int g = 0;

struct S mk(void)
    CERASTES_REQUIRES(cerastes_owns(g) && 0 <= g && g < 100)
    CERASTES_ENSURES(r, cerastes_owns(g) && g == cerastes_old(g) + 1 && r.x == g && r.y == 0)
{
  g += 1;
  struct S s = {g, 0};
  return s;
}

int h2(struct S a, struct S b)
{
  return a.x < b.x;
}

int main(void)
{
  return h2(mk(), mk());  // <- here
}

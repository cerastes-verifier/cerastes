// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0024 item 2: the closed expression forms of an assertion's condition are encoded as written and hold
#include <cerastes.h>

enum { LIM = 100, STEP = 3 };

static_assert(LIM * STEP == 300);
static_assert((unsigned char)255 == 255u);
static_assert(((1u << 31) >> 31) == 1u, "a shift to the top bit and back");
static_assert(4294967295u / 2u == 2147483647u, "unsigned division");
static_assert((7 > 3 ? 7 : 3) == 7, "a conditional over plain constants");
static_assert(-5 / 2 == -2 && -5 % 2 == -1, "division and remainder truncate");
static_assert(!(LIM == 0) && (LIM != 0 || STEP != 0), "logical operators");

int lim(void)
    CERASTES_ENSURES(r, r == 100)
{
  static_assert(LIM + STEP == 103, "block scope names enumerators too");
  return LIM;
}

int scaled(int x)
    CERASTES_REQUIRES(x >= 0 && x <= 715827882)
    CERASTES_ENSURES(r, r == x * STEP)
{
  return x * STEP;
}

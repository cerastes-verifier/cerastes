// expect: error overflow @ L9
// mode: c23, c++20
// twin: tests/scalar/static_assert_closed.c
// Pins: 0024 item 3: `enum { K = 2 }; static_assert(K == 2 && 65536u * 65536u == 0u)` is encoded as a whole
#include <cerastes.h>

enum { LIM = 100, STEP = 3 };

static_assert(LIM * STEP == 300 && 65536u * 65536u == 0u);
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
    CERASTES_REQUIRES(x >= 0 && x <= 7)
    CERASTES_ENSURES(r, r == x * STEP)
{
  return x * STEP;
}

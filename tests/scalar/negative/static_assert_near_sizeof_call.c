// expect: rejected cerastes-side_effect_not_evaluated @ L11
// mode: c23, c++20
// twin: tests/scalar/static_assert_closed.c
// Pins: 0024 item 3: `int g(void); static_assert(sizeof(g()) == sizeof(int) && 65536u * 65536u == 0u)`: a user call is not a skip trigger, so the assertion goes to the encoder,
// where 0020 item 5 rejects a call in sizeof (reject_sizeof_effect.c). The wrapping product beside it is not reached (a rejection stops the encoder)
#include <cerastes.h>

enum { LIM = 100, STEP = 3 };

int g(void);
static_assert(sizeof(g()) == sizeof(int) && 65536u * 65536u == 0u, "wraps");
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

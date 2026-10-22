// expect: verified
// expect: native-ok
// Pins: 0018 items 3, 5 and 7: a scoped enumeration is fixed (its values are the underlying type's), has no implicit conversions, and orders its own values
// Mode has a one-byte base (values 0 to 255); Plain has none, so its underlying type is int and every int is one of its values.
#include <cerastes.h>

enum class Mode : unsigned char { Off, On, Auto };
enum class Plain { P0, P1 };

Mode make_mode(int x)
    CERASTES_REQUIRES(0 <= x && x <= 255)
    CERASTES_ENSURES(r, static_cast<int>(r) == x)
{
  Mode m = static_cast<Mode>(x);
  return m;
}

int code(Mode m)
    CERASTES_ENSURES(r, (m == Mode::Off && r == 10) || (m == Mode::On && r == 20) || (m == Mode::Auto && r == 30) || (static_cast<int>(m) > 2 && r == 0))
{
  switch (m) {
  case Mode::Off:
    return 10;
  case Mode::On:
    return 20;
  case Mode::Auto:
    return 30;
  }
  return 0;
}

int ordered(Mode a, Mode b)
    CERASTES_ENSURES(r, (a < b && r == 1) || (a >= b && r == 0))
{
  if (a < b) {
    return 1;
  }
  return 0;
}

int plain_value(int x)
    CERASTES_ENSURES(r, r == x)
{
  Plain p = static_cast<Plain>(x);
  return static_cast<int>(p);
}

int demo(void)
    CERASTES_ENSURES(r, r == 316)
{
  Mode off = make_mode(0);
  Mode on = make_mode(1);
  Mode aut = make_mode(2);
  Mode top = make_mode(255);
  int a = code(off);
  int b = code(on);
  int c = code(aut);
  int d = code(top);
  int e = ordered(on, aut);
  int f = ordered(aut, on);
  int g = ordered(on, on);
  int h = static_cast<int>(top);
  return a + b + c + d + e + f + g + h;
}

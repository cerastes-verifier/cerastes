// expect: verified
// expect: native-ok
// mode: c++20
// Pins: 0019 item 11 (C++): scoped enumerations and named constants follow the same two rules
#include <cerastes.h>

enum class S { SA, SB };
enum color { RED, GREEN, BLUE };
constexpr color K = GREEN;

int scoped(S s)
    CERASTES_ENSURES(r, r == (s == S::SA ? 1 : (s == S::SB ? 2 : 0)))
{
  switch (s) {
  case S::SA:
    return 1;
  case static_cast<S>(1):
    return 2;
  }
  return 0;
}

int named(color c)
    CERASTES_ENSURES(r, r == (c == RED ? 1 : (c == GREEN ? 2 : (c == BLUE ? 3 : 0))))
{
  switch (c) {
  case RED:
    return 1;
  case K:
    return 2;
  case BLUE:
    return 3;
  }
  return 0;
}

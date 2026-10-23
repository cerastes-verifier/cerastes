// expect: rejected cerastes-self_assignment @ L10
// expect: rejected cerastes-self_assignment @ L17
// mode: c23, c++20
// twin: tests/scalar/self_assign_not_warned.c
// Pins: 0021 item 8 (clang -Wself-assign): a self-assignment cast to void, and one used in an initializer, are rejected
#include <cerastes.h>

int void_cast(int x)
{
  (void)(x = x);
  return x;
}

int initializer(int x)
    CERASTES_REQUIRES(x < 2147483647)
{
  int y = (x = x) + 1;
  return x + y;
}

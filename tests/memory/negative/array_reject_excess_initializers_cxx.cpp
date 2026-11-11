// expect: rejected too_many_initializer_values @ L10
// mode: c++20
// twin: tests/memory/array_sum_two.c
// Pins: 0059 item 3: in C++ more initializers than elements is EDG's own error (#146), before the back end runs (not lifted: EDG's own error)
// Pre-registered from record 0059 item 3
#include <cerastes.h>

int f(void)
{
  [[maybe_unused]] int a[3] = {1, 2, 3, 4};
  return 0;
}

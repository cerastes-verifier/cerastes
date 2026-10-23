// expect: rejected cerastes-self_assignment @ L10
// expect: rejected cerastes-self_assignment @ L17
// mode: c23, c++20
// twin: tests/scalar/self_assign_not_warned.c
// Pins: 0021 item 8 (clang -Wself-assign): a self-assignment in a do { ... } while (0) body, and of a local that has an initializer
#include <cerastes.h>

int do_body(int x)
{
  do { x = x; } while (0);
  return x;
}

int local_variable(void)
{
  int local = 1;
  local = local;
  return local;
}

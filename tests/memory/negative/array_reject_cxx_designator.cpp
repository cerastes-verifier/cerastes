// expect: rejected cerastes-c99_designator @ L11
// expect: rejected cerastes-c99_designator @ L17
// mode: c++20
// twin: tests/memory/array_init_partial.c
// Pins: 0059 item 3 (as updated): an array designator in C++ is rejected like the compilers (clang -Wc99-designator by default, GCC 16 refuses both forms); EDG accepts both silently (lifted by: never)
// Pre-registered from record 0059 item 3 (as updated)
#include <cerastes.h>

int one(void)
{
  [[maybe_unused]] int a[3] = {[2] = 5};
  return 0;
}

int two(void)
{
  [[maybe_unused]] int a[4] = {[2] = 5, 6};
  return 0;
}

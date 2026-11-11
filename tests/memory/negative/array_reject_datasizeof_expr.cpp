// expect: rejected cerastes-unsupported_type @ L11
// mode: c++20
// twin: tests/memory/array_sizeof_value.c
// Pins: 0059 item 11 (audit round 3, item 1): __datasizeof of an array local is rejected like __datasizeof of an array type (0024 item 6): only sizeof is lifted
// Pre-registered from audit round 3 of record 0059: the tag is the one of reject_datasizeof_array.cpp
#include <cerastes.h>

int size(void)
{
  int a[3] = {1, 2, 3};
  return (int)__datasizeof(a) + a[0];
}

// expect: rejected cerastes-array_address_null_test @ L16
// expect: rejected cerastes-array_address_null_test @ L25
// expect: rejected cerastes-array_address_null_test @ L31
// expect: rejected cerastes-array_address_null_test @ L37
// expect: rejected cerastes-array_address_null_test @ L43
// mode: c23, c++20
// twin: tests/memory/array_sum_two.c
// Pins: 0059 item 10: a truth test or a null comparison of an array's address (if (a), !a, a == NULL, a == 0, a != NULL) is rejected like the compilers (clang -Wpointer-bool-conversion and -Wtautological-pointer-compare by default, GCC -Waddress at -Wall)
// Pre-registered from record 0059 item 10
#include <cerastes.h>
#include <stddef.h>

int truth(void)
{
  int a[3] = {1, 2, 3};
  if (a) {
    return a[0];
  }
  return 0;
}

int negation(void)
{
  int a[3] = {1, 2, 3};
  return !a ? 0 : a[0];
}

int eq_null(void)
{
  int a[3] = {1, 2, 3};
  return (a == NULL) ? 0 : a[0];
}

int eq_zero(void)
{
  int a[3] = {1, 2, 3};
  return (a == 0) ? 0 : a[0];
}

int ne_null(void)
{
  int a[3] = {1, 2, 3};
  return (a != NULL) ? a[0] : 0;
}

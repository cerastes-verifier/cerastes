// expect: rejected cerastes-unsupported_annotation @ L11
// expect: rejected cerastes-unsupported_annotation @ L16
// expect: rejected cerastes-unsupported_type @ L21
// mode: c23
// c-only: restrict does not exist in C++20 (EDG, clang++ and g++ refuse it, decision 0037 item 2)
// twin: tests/memory/restrict_dropped.c
// Pins: 0064 items 13 and 19 (audit round R, note 8): a row parameter with restrict on the pointer (int (*restrict a)[3], int a[restrict][3]) is a restrict parameter and draws item 19's rejection at the function's head (unsupported_annotation); an array of restrict pointers as a parameter (int *restrict a[restrict 2]) has restrict on its pointee and stays unsupported_type at the function's line (0032 item 2)
// Pre-registered from record 0064 items 13 and 19 (Part E), before any run: rejected (tags by the record: unsupported_annotation, unsupported_annotation, unsupported_type)
#include <cerastes.h>

int star(int (*restrict a)[3])  // <- here
{
  return a[0][0] * 0;
}

int rows(int a[restrict][3])  // <- here
{
  return a[0][0] * 0;
}

int array_of_restrict(int *restrict a[restrict 2])  // <- here
{
  return *a[0] * 0;
}

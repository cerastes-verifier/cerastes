// expect: rejected cerastes-unsupported_annotation @ L8
// c-only: restrict does not exist in C++20 (EDG, clang++ and g++ refuse it, decision 0037 item 2)
// twin: tests/memory/restrict_owned.c
// Pins: 0037 items 2 and 15: restrict on a parameter of a function with no contract is rejected (re-expected by M8)
#include <cerastes.h>
#include <stddef.h>

int get(int *restrict p)
{
  return *p;
}

// expect: rejected cerastes-unsupported_annotation @ L8
// c-only: restrict does not exist in C++20 (EDG, clang++ and g++ refuse it, decision 0037 item 2)
// twin: tests/memory/restrict_dropped.c
// Pins: 0037 items 2 and 15: restrict on a parameter of a function with no contract is rejected (re-expected by M8) [0064 item 19, question 326: every restrict parameter is now rejected by that one rule, whatever the contract; this pin's row keeps its tag and line and its condition no longer matters]
#include <cerastes.h>
#include <stddef.h>

int get(int *restrict p)
{
  return *p;
}

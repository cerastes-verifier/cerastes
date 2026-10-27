// expect: rejected cerastes-truth_value_compare @ L12
// expect: frontend-warning unsigned_compare_with_zero @ L12
// mode: c23
// twin: tests/scalar/warn_accepted_neighbours.c
// c-only: EDG warns (#186) in C only; reject_warn_truth_bool_ge_zero_cpp.cpp is the C++ run
// Pins: 0025 item 6 R6: a bool compared with 0 (clang and gcc); EDG prints its #186 for it in C only (a bool is an unsigned type there), which is why the C and the C++ run are two files
#include <cerastes.h>
#include <stdbool.h>

int test(int a, int b, bool p, bool q)
{
  return p >= 0;
}

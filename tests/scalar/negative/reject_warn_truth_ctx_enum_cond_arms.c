// expect: rejected cerastes-int_in_truth_context @ L13
// mode: c23
// twin: tests/scalar/warn_accepted_neighbours.c
// c-only: in C++ a ?: of two enumerators has enumeration type, rejected as unsupported_expression (0018 item 8) before the truth context is looked at
// Pins: 0025 item 6 R7: a `?:` with an enumerator arm of value 2 as a condition (gcc); C only: in C++ the `?:` has enumeration type, which 0018 item 8 rejects (unsupported_expression)
#include <cerastes.h>
#include <stdbool.h>

enum e7 { Z7 = 0, ONE7 = 1, TWO7 = 2 };

int test(int c)
{
  if (c ? TWO7 : ONE7) {
    return 1;
  }
  return 0;
}

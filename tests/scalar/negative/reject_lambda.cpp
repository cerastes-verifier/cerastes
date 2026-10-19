// expect: rejected cerastes-unsupported_declaration @ L9
// twin: tests/scalar/if_else.c
// Pins: 0012 item 12: a lambda is rejected
// Assumption A3 (INDEX.md): one diagnostic, at the line of the lambda.
#include <cerastes.h>

int apply(int x)
{
  [[maybe_unused]] auto f = [](int y) { return y; };
  return x;
}

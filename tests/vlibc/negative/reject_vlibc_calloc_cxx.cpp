// expect: rejected cerastes-unsupported_expression @ L10
// twin: tests/vlibc/malloc_cstdlib.cpp
// Pins: 0050 item 7 (question 140): the typed view of vlibc_calloc is rejected in C++ for the same reason
// Pre-registered from record 0050 item 7: rejected unsupported_expression at the call
#include <vlibc/stdlib.h>
#include <cerastes.h>

int f()
{
  int *p = static_cast<int *>(vlibc_calloc(1, sizeof(int)));  // <- here
  vlibc_free(p);
  return 0;
}

// expect: rejected cerastes-unsupported_expression @ L10
// twin: tests/vlibc/malloc_cstdlib.cpp
// Pins: 0050 item 7 (question 140): C++20 creates no object in vlibc_malloc's storage ([intro.object]/10-13 name std::malloc and its family only), so the typed view is rejected in C++ until M7
// Pre-registered from record 0050 item 7: rejected unsupported_expression at the call
#include <vlibc/stdlib.h>
#include <cerastes.h>

int f()
{
  int *p = static_cast<int *>(vlibc_malloc(sizeof(int)));  // <- here
  vlibc_free(p);
  return 0;
}

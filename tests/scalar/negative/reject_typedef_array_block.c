// expect: rejected cerastes-unsupported_declaration @ L9
// mode: c23, c++20
// twin: tests/scalar/sizeof_value.c
// Pins: 0024 item 6: the block-scope typedef is diagnosed at its declaration (both compilers warn about an unused local typedef, which is why this is a rejection test)
#include <cerastes.h>

int block(void)
{
  typedef char buf_t[4];
  return 0;
}

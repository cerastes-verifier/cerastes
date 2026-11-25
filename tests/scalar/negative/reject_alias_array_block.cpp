// expect: rejected cerastes-unsupported_declaration @ L9
// twin: tests/scalar/sizeof_value.c
// Pins: 0063 item 14 (question 302): an unused block-scope using-alias of an array type stays rejected at the alias, like reject_typedef_array_block.c (both compilers warn at -Wall: -Wunused-local-typedef; the warning family is M8's sweep, record 0024 item 6's diagnostic kept)
// Pre-registered from record 0063 item 14 (Part D), before any run: rejected
#include <cerastes.h>

int block()
{
  using buf_t = char[4];  // <- here
  return 0;
}

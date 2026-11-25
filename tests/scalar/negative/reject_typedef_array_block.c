// expect: rejected cerastes-unsupported_declaration @ L9
// mode: c23, c++20
// twin: tests/scalar/sizeof_value.c
// Pins: 0063 item 14 (flip C of M4.5e1e1, question 302; its expect line is unchanged): an UNUSED block-scope typedef of an array type stays rejected at its declaration, like the compilers' -Wunused-local-typedefs at -Wall (the warning family is M8's sweep; an unused `typedef int T;` is accepted and listed there); a block-scope typedef that a declaration, cast or type operand uses is accepted (tests/memory/array_typedef_scopes.c)
#include <cerastes.h>

int block(void)
{
  typedef char buf_t[4];
  return 0;
}

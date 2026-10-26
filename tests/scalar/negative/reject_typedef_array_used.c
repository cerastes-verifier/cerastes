// expect: rejected cerastes-unsupported_declaration @ L8
// expect: rejected cerastes-unsupported_type @ L12
// mode: c23, c++20
// twin: tests/scalar/sizeof_value.c
// Pins: 0024 item 6: the record says so: a typedef is diagnosed at its declaration and again where its type is used inside a type operand
#include <cerastes.h>

typedef char buf_t[4];

int used(void)
{
  return (int)sizeof(buf_t);
}

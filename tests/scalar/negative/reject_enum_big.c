// expect: rejected cerastes-unsupported_declaration @ L9
// mode: c23, c++20
// twin: tests/scalar/enum_arith.c
// Pins: 0018 item 2: an enumerator outside int (C++: unsigned underlying type; C23: a type of its own) is rejected, at that enumerator
#include <cerastes.h>

enum Big {
  SMALL = 1,
  BIG = 3000000000
};

int ident(int a)
{
  return a;
}

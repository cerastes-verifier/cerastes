// expect: rejected cerastes-unsupported_expression @ L11
// c-only: C++ refuses += on an enumeration (error #511)
// twin: tests/scalar/enum_loop.c
// Pins: 0018 item 8: += on an enumeration variable computes in unsigned int (C only; C++ refuses it), so it is rejected
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

enum color bump(enum color c)
{
  c += 1;
  return c;
}

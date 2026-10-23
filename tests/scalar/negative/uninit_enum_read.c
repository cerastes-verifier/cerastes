// expect: error uninitialized @ L13
// mode: c23, c++20
// twin: tests/scalar/uninit_bool_enum.c
// Pins: 0019 item 1: an enumeration local compared before it is assigned
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

int pick(int a)
{
  enum color c;
  int n = 0;
  if (c == RED) {
    n = a;
  }
  return n;
}

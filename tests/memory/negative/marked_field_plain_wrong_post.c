// expect: error postcondition @ L18
// mode: c23, c++20
// twin: tests/memory/marked_field_plain.c
// Pins: 0058 item 4: the same program with a wrong ensures: the marker changes nothing about the ordinary postcondition
// Pre-registered from record 0058 item 4: one postcondition error at the return
#include <cerastes.h>
#include <stdlib.h>

struct S {
  int *q CERASTES_REF_TO_UNINIT;
  int k;
};

int get(void)
    CERASTES_ENSURES(r, r == 6)
{
  struct S s = { NULL, 5 };
  return s.k;  // <- here
}

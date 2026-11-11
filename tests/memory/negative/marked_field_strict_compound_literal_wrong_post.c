// expect: error postcondition @ L19
// c-only: the address of a compound literal is unsupported_expression in C++ (record 0044 item 7); a compound literal as a value is a GNU extension there
// verify-options: --strict-init
// twin: tests/memory/marked_field_strict_compound_literal.c
// Pins: 0058 item 4: the default checks stay under --strict-init: a wrong ensures
// Pre-registered from record 0058 item 4: one postcondition error at the return (c23)
#include <cerastes.h>
#include <stdlib.h>

struct pt {
  int *q CERASTES_REF_TO_UNINIT;
  int n;
};

int use(void)
    CERASTES_ENSURES(r, r == 1)
{
  struct pt s = (struct pt){ NULL, 0 };
  return s.n;  // <- here
}

// expect: rejected cerastes-uninit_ref_initialized @ L19
// c-only: the address of a compound literal is unsupported_expression in C++ (record 0044 item 7); a compound literal as a value is a GNU extension there
// verify-options: --strict-init
// twin: tests/memory/marked_field_strict_compound_literal.c
// Pins: 0058 item 4 (under --strict-init): the address of an initialized object as a compound literal's marked leaf is rejected (c23)
// Pre-registered from record 0058 item 4: rejected uninit_ref_initialized at the leaf
#include <cerastes.h>
#include <stdlib.h>

struct pt {
  int *q CERASTES_REF_TO_UNINIT;
  int n;
};

int f(void)
    CERASTES_ENSURES(r, r == 0)
{
  int x = 1;
  struct pt s = (struct pt){ &x, 0 };  // <- here
  return s.n;
}

// expect: rejected cerastes-unsupported_expression @ L12
// expect: rejected cerastes-unsupported_expression @ L20
// c-only: compound literals are C (C++ rejects them, record 0044 item 7)
// twin: tests/memory/compound_literal_effects_allowed.c
// Pins: 0044 item 7 (audit round 2, B1, question 308: the scalar form of the same hole, present since M4.5c2a): *&(int){x++} used as an object, with an element that writes, as the right side of an assignment or a store: rejected at the literal (p52b and p52h; clang warns -Wunsequenced on the first by default)
// Pre-registered from audit round 2 of record 0063 (B1, question 308), before any run: rejected
#include <cerastes.h>

int object(void)
{
  int x = 1;
  x = *&(int){x++};  // <- here
  return x - 1;
}

int cell(void)
{
  int x = 1;
  int *p = &x;
  *p = *&(int){(*p)++};  // <- here
  return x - 1;
}

// expect: error postcondition @ L19
// mode: c23, c++20
// twin: tests/memory/matrix_value_param.c
// Pins: 0064 item 12 (question 353, question 364's form): a RESULT whose members after the matrix are shifted by one cell against the ensures (r.a is right; r.b[0] is 7 where the ensures says 6, r.b[1] 8 for 7, r.c 9 for 8): the postcondition fails at the return; a one-cell misalignment of the matrix against the members beside it would meet r.b[0], r.b[1] and r.c and verify it
// Pre-registered from record 0064 item 12 as amended on 2026-11-26 (Part E, slice 4), before any run: postcondition at the return, once
#include <cerastes.h>

struct U {
  int a;
  int m[2][2];
  int b[2];
  int c;
};

struct U make(void)
    CERASTES_ENSURES(r, r.a == 1 && r.b[0] == 6 && r.b[1] == 7 && r.c == 8)
{
  struct U v = {1, {{2, 3}, {4, 6}}, {7, 8}, 9};
  return v;  // <- here
}

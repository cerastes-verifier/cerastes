// expect: error precondition @ L26
// expect: error uninitialized @ L39
// mode: c23, c++20
// twin: tests/memory/matrix_value_param.c
// Pins: 0064 item 12 (question 353, question 364's form): two failures of one call. shifted: the caller's values after v.a are shifted by one cell against the callee's requires (v.a is right, 1; v.b[0] is 7 where the callee needs 6, v.b[1] 8 for 7, v.c 9 for 8), so the precondition fails at the call; a misalignment of the matrix's cells against the members beside it by one cell (the matrix giving its last cell to b[0]) would meet b[0] and b[1] and c and verify it. unwritten: the caller leaves one cell of the matrix without a value (m[1][1]) and passes the structure, so the call fails uninitialized, the cell's flag row (every cell is passed)
// Pre-registered from record 0064 item 12 as amended on 2026-11-26 (Part E, slice 4), before any run: precondition at the first call, uninitialized at the second
#include <cerastes.h>

struct U {
  int a;
  int m[2][2];
  int b[2];
  int c;
};

int pick(struct U v)
    CERASTES_REQUIRES(v.a == 1 && v.b[0] == 6 && v.b[1] == 7 && v.c == 8)
    CERASTES_ENSURES(r, r == 22)
{
  return v.a + v.b[0] + v.b[1] + v.c;
}

int shifted(void)
{
  struct U v = {1, {{2, 3}, {4, 6}}, {7, 8}, 9};
  return pick(v);  // <- here
}

int unwritten(void)
{
  struct U v;
  v.a = 1;
  v.m[0][0] = 2;
  v.m[0][1] = 3;
  v.m[1][0] = 4;
  v.b[0] = 6;
  v.b[1] = 7;
  v.c = 8;
  return pick(v);  // <- here
}

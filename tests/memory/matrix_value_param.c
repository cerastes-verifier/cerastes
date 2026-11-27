// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 12 (question 353, question 364's form: a contract cannot read a cell of a matrix member of a structure value): a structure with a matrix member and members beside it (struct U { int a; int m[2][2]; int b[2]; int c; }, six counted cells: the four of m and the two of b, under the limit of 7) is passed by value with a distinct value in every cell, and a RESULT carries the same values: the callee's requires and the ensures state the members BESIDE the matrix (v.a, v.b[0], v.b[1], v.c), so a misalignment of the matrix's cells against them would be observed (the unsound direction: a requires on v.c met by a matrix cell); the caller reads the result's members beside the matrix back by checks. negative/matrix_value_param_shifted.c and negative/matrix_value_result_shifted.c are the twins
// Pre-registered from record 0064 item 12 as amended on 2026-11-26 (Part E, slice 4), before any run: verifies
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

int use(void)
    CERASTES_ENSURES(r, r == 22)
{
  struct U v = {1, {{2, 3}, {4, 5}}, {6, 7}, 8};
  return pick(v);
}

struct U make(void)
    CERASTES_ENSURES(r, r.a == 1 && r.b[0] == 6 && r.b[1] == 7 && r.c == 8)
{
  struct U v = {1, {{2, 3}, {4, 5}}, {6, 7}, 8};
  return v;
}

int use_result(void)
    CERASTES_ENSURES(r, r == 22)
{
  struct U w = make();
  CERASTES_CHECK(w.a == 1);
  CERASTES_CHECK(w.b[0] == 6);
  CERASTES_CHECK(w.b[1] == 7);
  CERASTES_CHECK(w.c == 8);
  return w.a + w.b[0] + w.b[1] + w.c;
}

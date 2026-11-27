// expect: rejected cerastes-unsupported_annotation @ L19
// expect: rejected cerastes-unsupported_annotation @ L25
// expect: rejected cerastes-unsupported_expression @ L35
// expect: rejected cerastes-unsupported_expression @ L40
// mode: c23, c++20
// twin: tests/memory/matrix_value_param.c
// Pins: 0064 item 12 (question 364, M4.5f3 with array values; fail closed): a contract cannot read a cell of a matrix member of a structure VALUE: v.m[1][0] of a by-value parameter in a requires and r.m[1][0] of a result in an ensures are unsupported_annotation; g().m[1][2] in code and in a check is unsupported_expression (0063 item 13 accepts g().a[1] for an array member of scalars, listed); the members beside the matrix are read in matrix_value_param.c
// Pre-registered from record 0064 item 12 as amended on 2026-11-26 (Part E, slice 4), before any run: the four rows, one per function (the tags are the record's; the lines are the contract's and the expression's)
#include <cerastes.h>

struct U {
  int a;
  int m[2][2];
  int b[2];
  int c;
};

int param(struct U v)
    CERASTES_REQUIRES(v.m[1][0] == 4)  // <- here
{
  return v.a;
}

struct U result(void)
    CERASTES_ENSURES(r, r.m[1][0] == 4)  // <- here
{
  struct U v = {1, {{2, 3}, {4, 5}}, {6, 7}, 8};
  return v;
}

struct U g(void);

int code(void)
{
  return g().m[1][1];  // <- here
}

int checked(void)
{
  CERASTES_CHECK(g().m[1][0] == 4);  // <- here
  return 0;
}

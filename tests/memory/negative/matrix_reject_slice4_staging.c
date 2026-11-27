// expect: rejected cerastes-unsupported_type @ L20
// expect: rejected cerastes-unsupported_type @ L25
// (L30, a structure with a matrix member as a by-value parameter, was rejected here until slice 4 lifted it: tests/memory/matrix_value_param.c, its `pick`; the line stays)
// (L35, the same structure as a by-value result, was rejected here until slice 4 lifted it: tests/memory/matrix_value_param.c, its `make`; the line stays)
// (L44, a compound literal assigned to a structure with a matrix member, was rejected here until slice 4 lifted it: tests/memory/matrix_value_limit_seven_copy.c, its `literal7`; the line stays)
// (L52, a whole-structure copy b = a of a structure with a matrix member, was rejected here until slice 4 lifted it: tests/memory/matrix_value_copy.c; the line stays)
// mode: c23, c++20
// twin: tests/memory/matrix_local.c
// Pins: Flips A (E) (plan section 9), slice 4 (Commit 6): the staging pin of Commit 5 had rows for a structure with a matrix member as a value (passed, returned, built by a compound literal and copied whole); slice 4 lifts those four rows (the programs are the positives named in the comments above) and the file stays for the two rows of depth 3: a local array of depth 3 (int c[2][3][4], L20) and a row parameter of depth 3 (int a[][2][3], L25), rejected until the next session builds depth 3 (record 0064 items 2 and 13); the file's value functions are accepted programs now and are not reached while the first two rows stop the front end
// Pre-registered from edg-backend's report 11, section GATES (A), before any run: unsupported_type at the declaration (the first two, the parameter and the result) or where the value is written (the literal, the copy)
#include <cerastes.h>

struct U {
  int m[2][3];
  int n;
};

int cube(void)
{
  int c[2][3][4];  // <- here
  c[0][0][0] = 1;
  return c[0][0][0];
}

int row3(int a[][2][3], int n)  // <- here
{
  return n;
}

int by_value(struct U u)  // <- here
{
  return u.n;
}

struct U make(void)  // <- here
{
  struct U u = {{{1, 2, 3}, {4, 5, 6}}, 7};
  return u;
}

int literal(void)
{
  struct U b = {{{0, 0, 0}, {0, 0, 0}}, 0};
  b = (struct U){{{1, 2, 3}, {4, 5, 6}}, 7};  // <- here
  return b.n;
}

int copy(void)
{
  struct U a = {{{1, 2, 3}, {4, 5, 6}}, 7};
  struct U b = {{{0, 0, 0}, {0, 0, 0}}, 0};
  b = a;  // <- here
  return b.n;
}

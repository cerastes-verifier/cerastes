// expect: rejected cerastes-unsupported_type @ L20
// expect: rejected cerastes-unsupported_type @ L25
// expect: rejected cerastes-unsupported_type @ L30
// expect: rejected cerastes-unsupported_type @ L35
// expect: rejected cerastes-unsupported_type @ L44
// expect: rejected cerastes-unsupported_type @ L52
// mode: c23, c++20
// twin: tests/memory/matrix_local.c
// Pins: M4.5e1e2 (fourth session), Commit 6 of this session retires this staging pin (plan section 10, Flips A (E)): in the build of Commit 5 the constructs of slice 4 are rejected at their C line behind the encoder's closed constant (kPartESlice4): a local array of depth 3 (int c[2][3][4]), a row parameter of depth 3 (int a[][2][3]), and a structure with a matrix member as a value (record 0064 items 2 and 12): passed by value, returned by value, built by a compound literal and copied whole (b = a;). Slice 4 builds them, and this file's rows then become the positives matrix_depth3.c, array_rows_param_depth3.c and matrix_value_*.c
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

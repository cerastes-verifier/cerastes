// expect: error permission @ L74
// expect: error permission @ L81
// expect: error permission @ L88
// expect: error permission @ L95
// expect: error permission @ L102
// expect: error permission @ L109
// expect: error permission @ L117
// expect: error postcondition @ L126
// mode: c23, c++20
// twin: tests/memory/array_rows_struct_elem_call.c
// Pins: 0064 items 3, 5 and 13 (audit E3a finding 8 (c); the auditor's d01 to d05, a14c, a18 and d01c, measured `permission` at the call and `postcondition`, both modes): a call whose range over structures with an array member and whose second object may be one object (a cell of v[0], the scalar member of v[1], v[0]'s array member as a range, v[1..2) against v[0..2), v[0] against v[0..2), one matrix twice) is `permission` at the call, a second range of three cells over an array of two likewise (the caller does not own w[0..3)), and the callee's written value of the cell is 2, not 7
// Pre-registered (test-mutator-10, Task 1/2, before any run): one `permission` at each call, one `postcondition` at the last return; right-value siblings: cell (r == 1 + 2 + 8, the value of v[1].a[1] is 2), scalar, row, ranges, element, matrices in tests/memory/array_rows_struct_elem_call.c (the same calls with n == 1 or another object)
#include <cerastes.h>

struct P {
  int n;
  int a[3];
};

struct U {
  int n;
  int m[2][3];
};

void two(struct P *p, int n, int *q)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && cerastes_owns(*q) && n > 0)
    CERASTES_ENSURES(cerastes_owns_range(p, n) && cerastes_owns(*q) && p[0].a[1] == 1 && *q == 2)
{
  (void)n;
  p[0].a[1] = 1;
  *q = 2;
}

void twor(struct P *p, int n, int *q, int m)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && cerastes_owns_range(q, m) && n > 0 && m > 0)
    CERASTES_ENSURES(cerastes_owns_range(p, n) && cerastes_owns_range(q, m) && p[0].a[0] == 1 && q[0] == 2)
{
  (void)n;
  (void)m;
  p[0].a[0] = 1;
  q[0] = 2;
}

void twop(struct P *p, int n, struct P *q, int m)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && cerastes_owns_range(q, m) && n > 0 && m > 0)
    CERASTES_ENSURES(cerastes_owns_range(p, n) && cerastes_owns_range(q, m) && p[0].a[2] == 1 && q[0].a[2] == 2)
{
  (void)n;
  (void)m;
  p[0].a[2] = 1;
  q[0].a[2] = 2;
}

void twos(struct P *p, int n, struct P *q)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && cerastes_owns(*q) && n > 0)
    CERASTES_ENSURES(cerastes_owns_range(p, n) && cerastes_owns(*q) && p[0].a[2] == 1 && q->a[2] == 2)
{
  (void)n;
  p[0].a[2] = 1;
  q->a[2] = 2;
}

void uu(struct U *p, struct U *q)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(q->m))
    CERASTES_ENSURES(cerastes_owns(*p) && cerastes_owns(q->m) && p->m[1][2] == 1 && q->m[1][2] == 2)
{
  p->m[1][2] = 1;
  q->m[1][2] = 2;
}

int over_cell(void)
{
  struct P v[2] = {{1, {2, 3, 4}}, {5, {6, 7, 8}}};
  two(v, 2, &v[0].a[1]);  // <- here
  return 0;
}

int over_scalar(void)
{
  struct P v[2] = {{1, {2, 3, 4}}, {5, {6, 7, 8}}};
  two(v, 2, &v[1].n);  // <- here
  return 0;
}

int over_row(void)
{
  struct P v[2] = {{1, {2, 3, 4}}, {5, {6, 7, 8}}};
  twor(v, 2, v[0].a, 3);  // <- here
  return 0;
}

int over_ranges(void)
{
  struct P v[2] = {{1, {2, 3, 4}}, {5, {6, 7, 8}}};
  twop(v, 2, &v[1], 1);  // <- here
  return 0;
}

int over_element(void)
{
  struct P v[2] = {{1, {2, 3, 4}}, {5, {6, 7, 8}}};
  twos(v, 2, &v[0]);  // <- here
  return 0;
}

int over_matrix(void)
{
  struct U u = {7, {{1, 2, 3}, {4, 5, 6}}};
  uu(&u, &u);  // <- here
  return 0;
}

int too_short(void)
{
  struct P v[2] = {{1, {2, 3, 4}}, {5, {6, 7, 8}}};
  int w[2] = {0, 0};
  twor(v, 2, w, 3);  // <- here
  return 0;
}

int wrong_value(void)
    CERASTES_ENSURES(r, r == 7)
{
  struct P v[2] = {{1, {2, 3, 4}}, {5, {6, 7, 8}}};
  two(v, 1, &v[1].a[1]);
  return v[1].a[1];  // <- here
}

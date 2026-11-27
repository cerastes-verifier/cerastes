// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 items 3, 5 and 13 (audit E3a finding 8 (c); the auditor's d01b to d05b and a14b to a17b controls): a call that holds a range over structures with an array member (owns_range(p, n)) beside a cell of an element's array member, a cell of its scalar member, an element's array member as a range, another range of structures, one element, and (matrices) owns(*p) beside owns(q->m): the objects are disjoint when the range is cut short (n == 1) or the second object is another one, so the callee's two permissions pair and each side keeps what its callee wrote
// Pre-registered (test-mutator-10, Task 1/2, before any run): verifies (the right-value sibling of negative/array_rows_struct_elem_call_overlap.c: the same calls with n == 1 or another object; the values are the callees' written values 1 and 2); time it first
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

int cell(void)
    CERASTES_ENSURES(r, r == 1 + 2 + 8)
{
  struct P v[2] = {{1, {2, 3, 4}}, {5, {6, 7, 8}}};
  two(v, 1, &v[1].a[1]);
  return v[0].a[1] + v[1].a[1] + v[1].a[2];
}

int scalar(void)
    CERASTES_ENSURES(r, r == 1 + 2)
{
  struct P v[2] = {{1, {2, 3, 4}}, {5, {6, 7, 8}}};
  two(v, 1, &v[1].n);
  return v[0].a[1] + v[1].n;
}

int row(void)
    CERASTES_ENSURES(r, r == 1 + 2)
{
  struct P v[2] = {{1, {2, 3, 4}}, {5, {6, 7, 8}}};
  twor(v, 1, v[1].a, 3);
  return v[0].a[0] + v[1].a[0];
}

int ranges(void)
    CERASTES_ENSURES(r, r == 1 + 2)
{
  struct P v[2] = {{1, {2, 3, 4}}, {5, {6, 7, 8}}};
  twop(v, 1, &v[1], 1);
  return v[0].a[2] + v[1].a[2];
}

int element(void)
    CERASTES_ENSURES(r, r == 1 + 2)
{
  struct P v[2] = {{1, {2, 3, 4}}, {5, {6, 7, 8}}};
  twos(v, 1, &v[1]);
  return v[0].a[2] + v[1].a[2];
}

int matrices(void)
    CERASTES_ENSURES(r, r == 1 + 2)
{
  struct U u = {7, {{1, 2, 3}, {4, 5, 6}}};
  struct U w = {7, {{1, 2, 3}, {4, 5, 6}}};
  uu(&u, &w);
  return u.m[1][2] + w.m[1][2];
}

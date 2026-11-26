// expect: error unsequenced @ L64
// expect: error unsequenced @ L70
// expect: error unsequenced @ L76
// expect: error unsequenced @ L82
// expect: error unsequenced @ L88
// mode: c23, c++20
// twin: tests/memory/range_struct_seq.c
// Pins: 0064 item 11 (audit round C1, finding 1; the sequencing rows of a call that holds a range over structures have no other check behind them, so each row is pinned): one function per row, each unsequenced at its expression, once. beside_read: wr(v, 3) owns the leaves of v[0..3) and v[1].x is read in the same expression (s01); beside_value: the same beside a by-value use of the element, takeP(v[1]) (s01e); overlap: wr(v, 2) and wr(&v[1], 2) own v[1] twice (s02a); nested_ranges: wrT over the nested structures &v[1].t and wrS over the outer range of v (s02e); nested_read: wrT over &v[1].t beside a read of that nested leaf, v[1].t.b (z03). The auditor made the sequencing rows `assert true` in the kept Viper of two probes: for two calls over overlapping ranges (s02a) Silicon then verified the program, and for a call beside a read (s01) only the probe's own postcondition still failed; so nothing but these rows rejects the unsequenced pair
// Pre-registered from record 0064 item 11 and the audit's finding 1 (its batches s and z: s01, s01e, s02a, s02e, z03, probe texts of its section 7), before any run: one unsequenced per function, at the line of its expression
#include <cerastes.h>
#include <stddef.h>

struct P {
  int x;
  int y;
};

struct T {
  int a;
  int b;
};

struct S {
  int x;
  struct T t;
};

int wr(struct P *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && n > 1)
    CERASTES_ENSURES(r, cerastes_owns_range(p, n) && p[1].x == 7 && r == 0)
{
  (void)n;
  p[1].x = 7;
  return 0;
}

int wrS(struct S *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && n > 1)
    CERASTES_ENSURES(r, cerastes_owns_range(p, n) && p[1].t.b == 7 && r == 0)
{
  (void)n;
  p[1].t.b = 7;
  return 0;
}

int wrT(struct T *q, int m)
    CERASTES_REQUIRES(cerastes_owns_range(q, m) && m > 0)
    CERASTES_ENSURES(r, cerastes_owns_range(q, m) && q[0].b == 5 && r == 0)
{
  (void)m;
  q[0].b = 5;
  return 0;
}

int takeP(struct P t)
    CERASTES_ENSURES(r, r == t.x)
{
  return t.x;
}

int beside_read(void)
{
  struct P v[3] = {{1, 2}, {3, 4}, {5, 6}};
  return wr(v, 3) + v[1].x;
}

int beside_value(void)
{
  struct P v[3] = {{1, 2}, {3, 4}, {5, 6}};
  return wr(v, 3) + takeP(v[1]);
}

int overlap(void)
{
  struct P v[3] = {{1, 2}, {3, 4}, {5, 6}};
  return wr(v, 2) + wr(&v[1], 2);
}

int nested_ranges(void)
{
  struct S v[3] = {{1, {2, 3}}, {1, {2, 4}}, {1, {2, 3}}};
  return wrT(&v[1].t, 1) + wrS(v, 3);
}

int nested_read(void)
{
  struct S v[3] = {{1, {2, 3}}, {1, {2, 4}}, {1, {2, 3}}};
  return wrT(&v[1].t, 1) + v[1].t.b;
}

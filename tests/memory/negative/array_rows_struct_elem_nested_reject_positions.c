// expect: rejected cerastes-unsupported_type @ L55
// expect: rejected cerastes-unsupported_expression @ L56
// expect: rejected cerastes-unsupported_type @ L62
// expect: rejected cerastes-unsupported_expression @ L63
// expect: rejected cerastes-unsupported_type @ L66
// expect: rejected cerastes-unsupported_annotation @ L69
// expect: rejected cerastes-unsupported_expression @ L71
// expect: rejected cerastes-unsupported_type @ L75
// expect: rejected cerastes-unsupported_expression @ L79
// expect: rejected cerastes-unsupported_type @ L84
// expect: rejected cerastes-unsupported_expression @ L85
// expect: rejected cerastes-unsupported_declaration @ L89
// expect: rejected cerastes-unsupported_annotation @ L107
// expect: rejected cerastes-unsupported_annotation @ L108
// expect: rejected cerastes-unsupported_expression @ L110
// expect: rejected cerastes-unsupported_annotation @ L115
// expect: rejected cerastes-unsupported_annotation @ L116
// expect: rejected cerastes-unsupported_expression @ L118
// expect: rejected cerastes-unsupported_annotation @ L122
// expect: rejected cerastes-unsupported_annotation @ L123
// expect: rejected cerastes-unsupported_expression @ L125
// expect: rejected cerastes-unsupported_type @ L131
// expect: rejected cerastes-unsupported_type @ L136
// expect: rejected cerastes-unsupported_declaration @ L139
// expect: rejected cerastes-unsupported_type @ L143
// expect: rejected cerastes-unsupported_declaration @ L146
// expect: rejected cerastes-unsupported_declaration @ L150
// expect: rejected cerastes-unsupported_expression @ L151
// expect: rejected cerastes-unsupported_type @ L158
// expect: rejected cerastes-unsupported_type @ L160
// expect: rejected cerastes-unsupported_type @ L163
// expect: frontend-warning set_but_not_used @ L55
// expect: frontend-warning set_but_not_used @ L66
// expect: frontend-warning set_but_not_used @ L84
// expect: frontend-warning set_but_not_used @ L101
// expect: frontend-warning set_but_not_used @ L150
// mode: c23, c++20
// twin: tests/memory/array_rows_struct_elem.c
// Pins: 0064 item 2 and question 382 (audit E3a finding 3; edg-backend report 17 section A, its table of the probes pA/n_*.c, rows both modes): an element structure that holds BY VALUE a structure with an array member (struct N { struct P p; int k; }, the array member standing one level down, v[i].p.a[j]) is rejected in every storage and at every position that computes the type: a local (declared, with an initializer), a global with internal and external linkage, a static local, a member array of a structure (at the member's declaration), a parameter's range under owns_range and reads_range, an array parameter, sizeof and _Alignof of the array type, a typedef of it (at the declaration, and its use as a computed type), typeof and a static assertion; each access row is the encoder's own `unsupported_expression` at the element's use; the neighbouring struct P (the array member standing directly in the element) is accepted everywhere (the controls of tests/memory/array_rows_struct_elem.c and the others of the family). M4.5e2
// Pre-registered (test-mutator-10, Task 1/2, before any run): rows registered from report 17 section A's table (tag and the probe line of each row mapped to this file's line: the probes' statements are kept as they are, one function per position; the member-form struct T2's two functions give no row of their own); measured in the combined form on the final join; corrected once after the first probe in lane edg (question 160, the program unchanged): the front end prints five `set_but_not_used` warnings (#550: the locals v, s, t, the second v and the global g1 are only written), identical in both modes, so the five `frontend-warning` rows are added and every other row moves down by the five header lines
#include <cerastes.h>

struct P {
  int n;
  int a[3];
};

struct N {
  struct P p;
  int k;
};

int f_local(void)
{
  struct N v[2];  // <- here
  v[1].p.a[1] = 3;  // <- here
  return 0;
}

int f_local_init(void)
{
  struct N v[2] = {{0}};  // <- here
  return v[1].p.a[1];  // <- here
}

static struct N g1[2];  // <- here

int f_global(void)
    CERASTES_REQUIRES(cerastes_owns(g1))  // <- here
{
  g1[1].p.a[1] = 3;  // <- here
  return 0;
}

struct N g2[2];  // <- here

int f_global_ext(void)
{
  return g2[1].p.a[1];  // <- here
}

int f_static(void)
{
  static struct N s[2];  // <- here
  s[1].p.a[1] = 3;  // <- here
  return 0;
}

struct T2 { struct N v[2]; int k; };  // <- here

int f_member(struct T2 *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(r, cerastes_owns(*p))
{
  p->v[1].p.a[1] = 3;
  return 0;
}

int f_member_local(void)
{
  struct T2 t;
  t.v[1].p.a[1] = 3;
  return 0;
}

int f_range(struct N *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && n > 1)  // <- here
    CERASTES_ENSURES(r, cerastes_owns_range(p, n))  // <- here
{
  p[1].p.a[1] = 3;  // <- here
  return 0;
}

int f_range_reads(struct N *p, int n)
    CERASTES_REQUIRES(cerastes_reads_range(p, n) && n > 1)  // <- here
    CERASTES_ENSURES(r, cerastes_reads_range(p, n))  // <- here
{
  return p[1].p.a[1];  // <- here
}

int f_param_array(struct N v[2])
    CERASTES_REQUIRES(cerastes_owns_range(v, 2))  // <- here
    CERASTES_ENSURES(r, cerastes_owns_range(v, 2))  // <- here
{
  v[1].p.a[1] = 3;  // <- here
  return 0;
}

int f_sizeof(void)
{
  return sizeof(struct N[2]) == sizeof(struct N) * 2;  // <- here
}

int f_alignof(void)
{
  return _Alignof(struct N[2]) == _Alignof(struct N);  // <- here
}

typedef struct N arr_t[2];  // <- here

int f_typedef(void)
{
  return sizeof(arr_t) == sizeof(struct N) * 2;  // <- here
}

typedef struct N arr2_t[2];  // <- here

int f_typedef_obj(void)
{
  arr2_t v;  // <- here
  v[1].p.a[1] = 3;  // <- here
  return 0;
}

int f_typeof(void)
{
  struct N one;
  __typeof__(one) v[2];  // <- here
  (void)v;
  return sizeof(__typeof__(struct N[2])) > 0;  // <- here
}

_Static_assert(sizeof(struct N[2]) > 0, "x");  // <- here

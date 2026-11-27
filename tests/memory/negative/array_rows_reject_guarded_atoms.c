// expect: rejected cerastes-unsupported_annotation @ L73
// expect: rejected cerastes-unsupported_annotation @ L74
// expect: rejected cerastes-unsupported_annotation @ L82
// expect: rejected cerastes-unsupported_annotation @ L83
// expect: rejected cerastes-unsupported_annotation @ L91
// expect: rejected cerastes-unsupported_annotation @ L92
// expect: rejected cerastes-unsupported_annotation @ L99
// expect: rejected cerastes-unsupported_annotation @ L100
// expect: rejected cerastes-unsupported_annotation @ L108
// expect: rejected cerastes-unsupported_annotation @ L109
// expect: rejected cerastes-unsupported_annotation @ L117
// expect: rejected cerastes-unsupported_annotation @ L118
// expect: rejected cerastes-unsupported_annotation @ L126
// expect: rejected cerastes-unsupported_annotation @ L127
// expect: rejected cerastes-unsupported_annotation @ L134
// expect: rejected cerastes-unsupported_annotation @ L135
// expect: rejected cerastes-unsupported_annotation @ L143
// expect: rejected cerastes-unsupported_annotation @ L144
// expect: rejected cerastes-unsupported_annotation @ L152
// expect: rejected cerastes-unsupported_annotation @ L153
// expect: rejected cerastes-unsupported_annotation @ L161
// expect: rejected cerastes-unsupported_annotation @ L162
// expect: rejected cerastes-unsupported_annotation @ L169
// expect: rejected cerastes-unsupported_annotation @ L170
// expect: rejected cerastes-unsupported_annotation @ L178
// expect: rejected cerastes-unsupported_annotation @ L179
// expect: rejected cerastes-unsupported_annotation @ L186
// expect: rejected cerastes-unsupported_annotation @ L187
// expect: rejected cerastes-unsupported_annotation @ L194
// expect: rejected cerastes-unsupported_annotation @ L195
// expect: rejected cerastes-unsupported_annotation @ L202
// expect: rejected cerastes-unsupported_annotation @ L203
// expect: rejected cerastes-unsupported_annotation @ L210
// expect: rejected cerastes-unsupported_annotation @ L211
// mode: c23, c++20
// twin: tests/memory/array_rows_guarded_atoms.c
// Pins: 0064 item 13 and question 384 (audit E3a finding 2 (c); edg-backend report 17 section D, probes pD/*.c and pD2/*.c): an `owns` or `reads` atom under a guard (`c == 0 || ...`, an arm of `?:`, `p == NULL || ...`) on a structure with a matrix member (struct U), the nested one (struct V), the member array of structures with an array member (struct T: the encoder's rule takes it on the shape it shares), a member of such a structure and its scalar member, and a global's whole-array atom (a global structure with a matrix member, a global matrix, a global array of structures with an array member, a global of struct T) is rejected at the atom in the requires and in the ensures (`unsupported_annotation`: the guarded well-formedness row of such an object, M4.5e2); one function per guard and atom form for each of the three structures (four forms each), then the globals and the auditor's m15 form with a read under the guard. The unguarded atoms (except the whole-array atom of a global array of struct P: it verifies, but the Viper lint refuses it beside the declarations of the other structures, the table collision of audit finding 2 (e), so it has no positive) and Part D's guarded struct P (the member array of scalars) verify: tests/memory/array_rows_guarded_atoms.c
// Pre-registered (test-mutator-10, Task 1/2, before any run): rows registered from report 17 section D's table (two rows per function, the requires and the ensures line, both modes; the global forms and u_ownsrowread from pD2; the global of struct T (global_member_array) is the report's pD/gt_or_c_owns.c form measured on a probe build, not in the table's edg44 rows: a guess to confirm at the final join); measured in the combined form
#include <cerastes.h>
#include <stddef.h>

struct U {
  int n;
  int m[2][3];
};

struct V {
  struct U u;
  int k;
};

struct W {
  int a[4];
  int n;
};

struct T {
  struct W v[2];
  int k;
};

struct P {
  int n;
  int a[3];
};

static struct U gu;
static int gm[2][3];
static struct P g[2];
static struct T gt;

int u_or_c_owns(struct U *p, int c)
    CERASTES_REQUIRES((c == 0 || cerastes_owns(*p)))  // <- here
    CERASTES_ENSURES(r, (c == 0 || cerastes_owns(*p)))  // <- here
{
  (void)p;
  (void)c;
  return 0;
}

int u_cond_ownsm(struct U *p, int c)
    CERASTES_REQUIRES((c ? cerastes_owns(p->m) : 1))  // <- here
    CERASTES_ENSURES(r, (c ? cerastes_owns(p->m) : 1))  // <- here
{
  (void)p;
  (void)c;
  return 0;
}

int u_or_null_reads(struct U *p, int c)
    CERASTES_REQUIRES((p == NULL || cerastes_reads(*p)))  // <- here
    CERASTES_ENSURES(r, (p == NULL || cerastes_reads(*p)))  // <- here
{
  (void)c;
  return 0;
}

int u_or_c_ownsn(struct U *p, int c)
    CERASTES_REQUIRES((c == 0 || cerastes_owns(p->n)))  // <- here
    CERASTES_ENSURES(r, (c == 0 || cerastes_owns(p->n)))  // <- here
{
  (void)p;
  (void)c;
  return 0;
}

int v_or_c_owns(struct V *p, int c)
    CERASTES_REQUIRES((c == 0 || cerastes_owns(*p)))  // <- here
    CERASTES_ENSURES(r, (c == 0 || cerastes_owns(*p)))  // <- here
{
  (void)p;
  (void)c;
  return 0;
}

int v_cond_ownsm(struct V *p, int c)
    CERASTES_REQUIRES((c ? cerastes_owns(p->u.m) : 1))  // <- here
    CERASTES_ENSURES(r, (c ? cerastes_owns(p->u.m) : 1))  // <- here
{
  (void)p;
  (void)c;
  return 0;
}

int v_or_null_reads(struct V *p, int c)
    CERASTES_REQUIRES((p == NULL || cerastes_reads(*p)))  // <- here
    CERASTES_ENSURES(r, (p == NULL || cerastes_reads(*p)))  // <- here
{
  (void)c;
  return 0;
}

int v_or_c_ownsn(struct V *p, int c)
    CERASTES_REQUIRES((c == 0 || cerastes_owns(p->k)))  // <- here
    CERASTES_ENSURES(r, (c == 0 || cerastes_owns(p->k)))  // <- here
{
  (void)p;
  (void)c;
  return 0;
}

int t_or_c_owns(struct T *p, int c)
    CERASTES_REQUIRES((c == 0 || cerastes_owns(*p)))  // <- here
    CERASTES_ENSURES(r, (c == 0 || cerastes_owns(*p)))  // <- here
{
  (void)p;
  (void)c;
  return 0;
}

int t_cond_ownsm(struct T *p, int c)
    CERASTES_REQUIRES((c ? cerastes_owns(p->v) : 1))  // <- here
    CERASTES_ENSURES(r, (c ? cerastes_owns(p->v) : 1))  // <- here
{
  (void)p;
  (void)c;
  return 0;
}

int t_or_null_reads(struct T *p, int c)
    CERASTES_REQUIRES((p == NULL || cerastes_reads(*p)))  // <- here
    CERASTES_ENSURES(r, (p == NULL || cerastes_reads(*p)))  // <- here
{
  (void)c;
  return 0;
}

int t_or_c_ownsn(struct T *p, int c)
    CERASTES_REQUIRES((c == 0 || cerastes_owns(p->k)))  // <- here
    CERASTES_ENSURES(r, (c == 0 || cerastes_owns(p->k)))  // <- here
{
  (void)p;
  (void)c;
  return 0;
}

int global_struct(int c)
    CERASTES_REQUIRES(c == 0 || cerastes_owns(gu))  // <- here
    CERASTES_ENSURES(r, c == 0 || cerastes_owns(gu))  // <- here
{
  (void)c;
  return gu.n * 0;
}

int global_matrix(int c)
    CERASTES_REQUIRES(c == 0 || cerastes_owns(gm))  // <- here
    CERASTES_ENSURES(r, c == 0 || cerastes_owns(gm))  // <- here
{
  (void)c;
  return gm[0][0] * 0;
}

int global_array(int c)
    CERASTES_REQUIRES(c == 0 || cerastes_owns(g))  // <- here
    CERASTES_ENSURES(r, c == 0 || cerastes_owns(g))  // <- here
{
  (void)c;
  return g[0].n * 0;
}

int global_member_array(int c)
    CERASTES_REQUIRES(c == 0 || cerastes_owns(gt))  // <- here
    CERASTES_ENSURES(r, c == 0 || cerastes_owns(gt))  // <- here
{
  (void)c;
  return gt.k * 0;
}

int cell_read_under_guard(struct U *p, int c)
    CERASTES_REQUIRES((c == 0 || cerastes_owns(*p)) && (c == 0 || p->m[1][2] == 0))  // <- here
    CERASTES_ENSURES(r, (c == 0 || cerastes_owns(*p)) && (c == 0 || p->m[1][2] == 0))  // <- here
{
  (void)p;
  (void)c;
  return 0;
}

// expect: verified
// expect: native-ok
// mode: c23, c++20
// twin-exempt: controls of rejection pins: nothing in a function that only states an atom can fail; the must-fail forms are the rejected pins of negative/array_rows_reject_guarded_atoms.c
// Pins: 0064 item 13 and question 384 (edg-backend report 17 section D, the unguarded controls pD3/*.c and Part D's guarded struct P forms pD/p_*.c): the neighbours of the rejected guarded atoms of negative/array_rows_reject_guarded_atoms.c verify: the same four atoms unguarded on a structure with a matrix member, the nested one and the member array of structures with an array member, the global forms, and the guarded atoms (`c == 0 || owns(*p)`, `c ? owns(p->n) : 1`, `p == NULL || reads(*p)`) on struct P, whose array member holds scalars (Part D); the unguarded whole-array atom of a global array of struct P is NOT here: with the other structures of this unit declared beside it the lint refuses its Viper (audit finding 2 (e), the table collision), and that case is a RUN FINDING of test-mutator-10 (state file)
// Pre-registered (test-mutator-10, Task 1/2, before any run): verifies (every function is a contract that nothing proves or breaks: the pin is that the encoder and the lint accept the atom); time it first; corrected once after the first probe in lane edg (question 160, program only, no expect line): in p_or_c_owns and p_cond_ownsn `(void)p;` is a use of a pointer the function does not own when the guard is false (`lifetime`), so p is `[[maybe_unused]]` there
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
static struct T gt;

int u_owns(struct U *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(r, cerastes_owns(*p))
{
  (void)p;
  return 0;
}

int u_owns_member(struct U *p)
    CERASTES_REQUIRES(cerastes_owns(p->m))
    CERASTES_ENSURES(r, cerastes_owns(p->m))
{
  (void)p;
  return 0;
}

int u_reads(struct U *p)
    CERASTES_REQUIRES(cerastes_reads(*p))
    CERASTES_ENSURES(r, cerastes_reads(*p))
{
  (void)p;
  return 0;
}

int u_owns_scalar(struct U *p)
    CERASTES_REQUIRES(cerastes_owns(p->n))
    CERASTES_ENSURES(r, cerastes_owns(p->n))
{
  (void)p;
  return 0;
}

int v_owns(struct V *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(r, cerastes_owns(*p))
{
  (void)p;
  return 0;
}

int v_owns_member(struct V *p)
    CERASTES_REQUIRES(cerastes_owns(p->u.m))
    CERASTES_ENSURES(r, cerastes_owns(p->u.m))
{
  (void)p;
  return 0;
}

int v_reads(struct V *p)
    CERASTES_REQUIRES(cerastes_reads(*p))
    CERASTES_ENSURES(r, cerastes_reads(*p))
{
  (void)p;
  return 0;
}

int v_owns_scalar(struct V *p)
    CERASTES_REQUIRES(cerastes_owns(p->k))
    CERASTES_ENSURES(r, cerastes_owns(p->k))
{
  (void)p;
  return 0;
}

int t_owns(struct T *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(r, cerastes_owns(*p))
{
  (void)p;
  return 0;
}

int t_owns_member(struct T *p)
    CERASTES_REQUIRES(cerastes_owns(p->v))
    CERASTES_ENSURES(r, cerastes_owns(p->v))
{
  (void)p;
  return 0;
}

int t_reads(struct T *p)
    CERASTES_REQUIRES(cerastes_reads(*p))
    CERASTES_ENSURES(r, cerastes_reads(*p))
{
  (void)p;
  return 0;
}

int t_owns_scalar(struct T *p)
    CERASTES_REQUIRES(cerastes_owns(p->k))
    CERASTES_ENSURES(r, cerastes_owns(p->k))
{
  (void)p;
  return 0;
}

int global_struct(void)
    CERASTES_REQUIRES(cerastes_owns(gu))
    CERASTES_ENSURES(r, cerastes_owns(gu))
{
  return gu.n * 0;
}

int global_matrix(void)
    CERASTES_REQUIRES(cerastes_owns(gm))
    CERASTES_ENSURES(r, cerastes_owns(gm))
{
  return gm[0][0] * 0;
}

int global_member_array(void)
    CERASTES_REQUIRES(cerastes_owns(gt))
    CERASTES_ENSURES(r, cerastes_owns(gt))
{
  return gt.k * 0;
}

int p_or_c_owns([[maybe_unused]] struct P *p, int c)
    CERASTES_REQUIRES((c == 0 || cerastes_owns(*p)))
    CERASTES_ENSURES(r, (c == 0 || cerastes_owns(*p)))
{
  (void)c;
  return 0;
}

int p_cond_ownsn([[maybe_unused]] struct P *p, int c)
    CERASTES_REQUIRES((c ? cerastes_owns(p->n) : 1))
    CERASTES_ENSURES(r, (c ? cerastes_owns(p->n) : 1))
{
  (void)c;
  return 0;
}

int p_or_null_reads(struct P *p, int c)
    CERASTES_REQUIRES((p == NULL || cerastes_reads(*p)))
    CERASTES_ENSURES(r, (p == NULL || cerastes_reads(*p)))
{
  (void)p;
  (void)c;
  return 0;
}

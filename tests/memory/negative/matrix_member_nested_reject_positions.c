// expect: rejected cerastes-unsupported_declaration @ L51
// expect: rejected cerastes-unsupported_declaration @ L56
// expect: rejected cerastes-unsupported_annotation @ L61
// expect: rejected cerastes-unsupported_expression @ L67
// expect: rejected cerastes-unsupported_expression @ L71
// expect: rejected cerastes-unsupported_expression @ L77
// expect: rejected cerastes-unsupported_expression @ L81
// expect: rejected cerastes-unsupported_annotation @ L86
// expect: rejected cerastes-unsupported_expression @ L88
// expect: rejected cerastes-unsupported_annotation @ L92
// expect: rejected cerastes-unsupported_annotation @ L93
// expect: rejected cerastes-unsupported_annotation @ L106
// expect: rejected cerastes-unsupported_annotation @ L113
// expect: rejected cerastes-unsupported_annotation @ L120
// expect: rejected cerastes-unsupported_annotation @ L125
// expect: rejected cerastes-unsupported_annotation @ L128
// expect: rejected cerastes-unsupported_annotation @ L131
// expect: rejected cerastes-unsupported_annotation @ L137
// expect: rejected cerastes-unsupported_annotation @ L147
// expect: rejected cerastes-unsupported_annotation @ L155
// expect: rejected cerastes-unsupported_annotation @ L163
// expect: rejected cerastes-unsupported_annotation @ L167
// expect: rejected cerastes-unsupported_annotation @ L176
// expect: rejected cerastes-unsupported_annotation @ L188
// expect: rejected cerastes-unsupported_annotation @ L196
// mode: c23, c++20
// twin: tests/memory/matrix_member_nested.c
// Pins: 0064 item 15's last clause and question 348 (M4.5e2), negative/matrix_member_reject_positions.c for the NESTED structure type: struct V { struct U u; int k; } with struct U { int n; int m[2][3]; } (a structure whose by-value member holds the matrix) stays rejected at Part D's six positions, one row each: struct V beside an owning and a ref_to_uninit member and an owning member that points to V; malloc, calloc, free and owns_block of one (the free rows twice); ref_to_uninit, must_init and owning on a struct V * and an owning result; a partnerless ensures atom (the fresh result, owns(p->u.m)); a place through a heap read (owns(*p->q), p->q->u.m[1][2]); a written owns(*p) in a loop invariant; and, after call_fresh, an owning global, an owning local, owns(p->u.m) and reads(*p) written in loop invariants and a partnerless owns(*p) on a plain pointer. The tags and lines are what edg36 (il154) printed for edg-backend's probe files m348v.c and m348vb.c in both modes (edg-backend report 14, section 348 for struct V); the combined file has not been probed
// Pre-registered from edg-backend's report 14, section 348 for struct V, before any run: rejected (observed on the two probe files separately, not predicted; the combined file has not been probed)
#include <cerastes.h>
#include <stddef.h>
#include <stdlib.h>


struct U {
  int n;
  int m[2][3];
};

struct V {
  struct U u;
  int k;
};

struct R {
  struct V *q;
  int z;
};

struct OwnBeside {
  struct V v;  // <- here
  int *p CERASTES_OWNING;
};

struct MarkBeside {
  struct V v;  // <- here
  int *p CERASTES_REF_TO_UNINIT;
};

struct OwnT {
  CERASTES_OWNING struct V *q;  // <- here
  int z;
};

int f_malloc(void)
{
  struct V *p = (struct V *)malloc(sizeof(struct V));  // <- here
  if (p == NULL) {
    return 0;
  }
  free(p);  // <- here
  return 1;
}

int f_calloc(void)
{
  struct V *p = (struct V *)calloc(1, sizeof(struct V));  // <- here
  if (p == NULL) {
    return 0;
  }
  free(p);  // <- here
  return 1;
}

void f_free(struct V *p)
    CERASTES_REQUIRES(cerastes_owns_block(p))  // <- here
{
  free(p);  // <- here
}

void f_borrow(struct V *p)
    CERASTES_REQUIRES(cerastes_owns_block(p))  // <- here
    CERASTES_ENSURES(cerastes_owns_block(p))  // <- here
{
  (void)p;
}

int use_own(struct OwnBeside *a, struct MarkBeside *b, struct OwnT *c)
{
  (void)a;
  (void)b;
  (void)c;
  return 0;
}

void f_ref(struct V *p CERASTES_REF_TO_UNINIT)  // <- here
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  (void)p;
}

void f_must(struct V *p CERASTES_MUST_INIT)  // <- here
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  (void)p;
}

void f_owning(CERASTES_OWNING struct V *p)  // <- here
{
  (void)p;
}

CERASTES_OWNING struct V *f_result(void);  // <- here

struct V *f_fresh(void)
    CERASTES_ENSURES(r, cerastes_owns(*r));  // <- here

int through_load_atom(struct R *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(*p->q))  // <- here
{
  return p->z;
}

int through_load_read(struct R *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && p->q->u.m[1][2] > 0)  // <- here
{
  return p->z;
}

void loop_atom(struct V *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 2 && cerastes_owns(*p))  // <- here
  CERASTES_DECREASES(2 - i)
  while (i < 2) {
    i = i + 1;
  }
}

void give(struct V *p)
    CERASTES_ENSURES(cerastes_owns(p->u.m));  // <- here

int call_fresh(void)
{
  struct V *p = f_fresh();
  return p == NULL;
}

CERASTES_OWNING static struct V *gp = NULL;  // <- here

int owning_local(void)
{
  CERASTES_OWNING struct V *l = NULL;  // <- here
  return l == NULL;
}

void loop_member(struct V *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 2 && cerastes_owns(p->u.m))  // <- here
  CERASTES_DECREASES(2 - i)
  while (i < 2) {
    i = i + 1;
  }
}

void loop_reads(struct V *p)
    CERASTES_REQUIRES(cerastes_reads(*p))
    CERASTES_ENSURES(cerastes_reads(*p))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 2 && cerastes_reads(*p))  // <- here
  CERASTES_DECREASES(2 - i)
  while (i < 2) {
    i = i + 1;
  }
}

void give_whole(struct V *p)
    CERASTES_ENSURES(cerastes_owns(*p));  // <- here

int use_gp(void)
{
  return gp == NULL;
}

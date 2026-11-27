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
// twin: tests/memory/array_rows_struct_elem_member.c
// Pins: 0064 item 15's last clause and question 348 (M4.5e2), negative/matrix_member_nested_reject_positions.c for the member array of structures with an array member: struct T { struct W v[2]; int k; } with struct W { int a[4]; int n; } stays rejected at Part D's six positions, one row each: struct T beside an owning and a ref_to_uninit member and an owning member that points to T; malloc, calloc, free and owns_block of one (the free rows twice); ref_to_uninit, must_init and owning on a struct T * and an owning result; a partnerless ensures atom (the fresh result, owns(p->v)); a place through a heap read (owns(*p->q), p->q->v[1].a[2]); a written owns(*p) in a loop invariant; and, after call_fresh, an owning global, an owning local, owns(p->v) and reads(*p) written in loop invariants and a partnerless owns(*p) on a plain pointer. The tags and lines are what edg40 (il178) printed for edg-backend's probe files m348t.c and m348tb.c in both modes (edg-backend report 16, section 348 for struct T); the combined file has not been probed
// Pre-registered from edg-backend's report 16, section 348 for struct T, before any run: rejected (observed on the two probe files separately, not predicted; the combined file has not been probed)
#include <cerastes.h>
#include <stddef.h>
#include <stdlib.h>


struct W {
  int a[4];
  int n;
};

struct T {
  struct W v[2];
  int k;
};

struct R {
  struct T *q;
  int z;
};

struct OwnBeside {
  struct T v;  // <- here
  int *p CERASTES_OWNING;
};

struct MarkBeside {
  struct T v;  // <- here
  int *p CERASTES_REF_TO_UNINIT;
};

struct OwnT {
  CERASTES_OWNING struct T *q;  // <- here
  int z;
};

int f_malloc(void)
{
  struct T *p = (struct T *)malloc(sizeof(struct T));  // <- here
  if (p == NULL) {
    return 0;
  }
  free(p);  // <- here
  return 1;
}

int f_calloc(void)
{
  struct T *p = (struct T *)calloc(1, sizeof(struct T));  // <- here
  if (p == NULL) {
    return 0;
  }
  free(p);  // <- here
  return 1;
}

void f_free(struct T *p)
    CERASTES_REQUIRES(cerastes_owns_block(p))  // <- here
{
  free(p);  // <- here
}

void f_borrow(struct T *p)
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

void f_ref(struct T *p CERASTES_REF_TO_UNINIT)  // <- here
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  (void)p;
}

void f_must(struct T *p CERASTES_MUST_INIT)  // <- here
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  (void)p;
}

void f_owning(CERASTES_OWNING struct T *p)  // <- here
{
  (void)p;
}

CERASTES_OWNING struct T *f_result(void);  // <- here

struct T *f_fresh(void)
    CERASTES_ENSURES(r, cerastes_owns(*r));  // <- here

int through_load_atom(struct R *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(*p->q))  // <- here
{
  return p->z;
}

int through_load_read(struct R *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && p->q->v[1].a[2] > 0)  // <- here
{
  return p->z;
}

void loop_atom(struct T *p)
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

void give(struct T *p)
    CERASTES_ENSURES(cerastes_owns(p->v));  // <- here

int call_fresh(void)
{
  struct T *p = f_fresh();
  return p == NULL;
}

CERASTES_OWNING static struct T *gp = NULL;  // <- here

int owning_local(void)
{
  CERASTES_OWNING struct T *l = NULL;  // <- here
  return l == NULL;
}

void loop_member(struct T *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 2 && cerastes_owns(p->v))  // <- here
  CERASTES_DECREASES(2 - i)
  while (i < 2) {
    i = i + 1;
  }
}

void loop_reads(struct T *p)
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

void give_whole(struct T *p)
    CERASTES_ENSURES(cerastes_owns(*p));  // <- here

int use_gp(void)
{
  return gp == NULL;
}

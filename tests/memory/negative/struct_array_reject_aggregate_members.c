// expect: rejected cerastes-unsupported_declaration @ L45
// expect: rejected cerastes-unsupported_declaration @ L50
// expect: rejected cerastes-unsupported_annotation @ L55
// expect: rejected cerastes-unsupported_expression @ L61
// expect: rejected cerastes-unsupported_expression @ L65
// expect: rejected cerastes-unsupported_expression @ L71
// expect: rejected cerastes-unsupported_expression @ L75
// expect: rejected cerastes-unsupported_annotation @ L80
// expect: rejected cerastes-unsupported_expression @ L82
// expect: rejected cerastes-unsupported_annotation @ L86
// expect: rejected cerastes-unsupported_annotation @ L87
// expect: rejected cerastes-unsupported_annotation @ L100
// expect: rejected cerastes-unsupported_annotation @ L107
// expect: rejected cerastes-unsupported_annotation @ L114
// expect: rejected cerastes-unsupported_annotation @ L119
// expect: rejected cerastes-unsupported_annotation @ L122
// expect: rejected cerastes-unsupported_annotation @ L125
// expect: rejected cerastes-unsupported_annotation @ L131
// expect: rejected cerastes-unsupported_annotation @ L141
// expect: rejected cerastes-unsupported_annotation @ L149
// mode: c23, c++20
// twin: tests/memory/struct_array_aggregate_ok.c
// Pins: 0064 item 15's last clause and question 348 (M4.5e2): a structure whose array member holds structures (struct T { struct P v[2]; int n; }) stays rejected at Part D's six positions, one row each: malloc, calloc, free and owns_block of it (L61 to L87: the free rows twice, as negative/struct_array_reject_heap.c has them); an owning and a ref_to_uninit member beside the member array (L45, L50 at the array member's declaration; L55 the owning member that points to T); ref_to_uninit, must_init and owning on a struct T * (L100, L107, L114) and an owning result of one (L119); a partnerless ensures atom on one (L122 beside a caller, L149 owns(p->v)); a place through a heap read (L125 owns(*p->q), L131 p->q->v[0].x); a written atom in a loop invariant (L141). The tags and lines are what the frozen base build (493a923) printed in both modes; Part D keeps tag and line and changes only the message
// Pre-registered from the frozen base build's R1 run (build/m4.5e1e2c/s0/tpin/t1.c, cerastes verify in both modes), before any part of Part D: rejected (observed, not predicted)
#include <cerastes.h>
#include <stddef.h>
#include <stdlib.h>

struct P {
  int x;
  int y;
};

struct T {
  struct P v[2];
  int n;
};

struct R {
  struct T *q;
  int z;
};

struct OwnBeside {
  struct P v[2];  // <- here
  int *p CERASTES_OWNING;
};

struct MarkBeside {
  struct P v[2];  // <- here
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
    CERASTES_REQUIRES(cerastes_owns(*p) && p->q->v[0].x > 0)  // <- here
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

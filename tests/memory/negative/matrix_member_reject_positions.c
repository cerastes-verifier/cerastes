// expect: rejected cerastes-unsupported_declaration @ L46
// expect: rejected cerastes-unsupported_declaration @ L51
// expect: rejected cerastes-unsupported_annotation @ L56
// expect: rejected cerastes-unsupported_expression @ L62
// expect: rejected cerastes-unsupported_expression @ L66
// expect: rejected cerastes-unsupported_expression @ L72
// expect: rejected cerastes-unsupported_expression @ L76
// expect: rejected cerastes-unsupported_annotation @ L81
// expect: rejected cerastes-unsupported_expression @ L83
// expect: rejected cerastes-unsupported_annotation @ L87
// expect: rejected cerastes-unsupported_annotation @ L88
// expect: rejected cerastes-unsupported_annotation @ L101
// expect: rejected cerastes-unsupported_annotation @ L108
// expect: rejected cerastes-unsupported_annotation @ L115
// expect: rejected cerastes-unsupported_annotation @ L120
// expect: rejected cerastes-unsupported_annotation @ L123
// expect: rejected cerastes-unsupported_annotation @ L126
// expect: rejected cerastes-unsupported_annotation @ L132
// expect: rejected cerastes-unsupported_annotation @ L142
// expect: rejected cerastes-unsupported_annotation @ L150
// expect: rejected cerastes-unsupported_annotation @ L158
// expect: rejected cerastes-unsupported_annotation @ L162
// expect: rejected cerastes-unsupported_annotation @ L171
// expect: rejected cerastes-unsupported_annotation @ L183
// expect: rejected cerastes-unsupported_annotation @ L191
// mode: c23, c++20
// twin: tests/memory/matrix_member.c
// Pins: 0064 item 15's last clause and question 348 (M4.5e2), the matrix member's counterpart of negative/struct_array_reject_aggregate_members.c and struct_array_reject_aggregate_owners.c: a structure with a MATRIX member (struct U { int n; int m[2][3]; }) stays rejected at Part D's six positions, one row each: an owning and a ref_to_uninit member beside the matrix member (L46, L51) and an owning member that points to U (L56); malloc, calloc, free and owns_block of it (L62 to L88: the free rows twice, as the member-array file has them); ref_to_uninit, must_init and owning on a struct U * (L101, L108, L115) and an owning result of one (L120); a partnerless ensures atom on one (L123 beside the fresh result, L150 owns(p->m)); a place through a heap read (L126 owns(*p->q), L132 p->q->m[1][2]); a written atom in a loop invariant (L142); and, in the functions after call_fresh (edg-backend's probe m348b.c), an owning global (L158), an owning local (L162), owns(p->m) and reads(*p) written in loop invariants (L171, L183) and a partnerless owns(*p) on a plain pointer (L191). The tags and lines are what the 3b-1 build (edg25, il85) printed for the two probe files in both modes (edg-backend report 12, section 348; the c23 and c++20 rows are identical); the combined file is to be re-probed on 3b-2's snapshot before its first run
// Pre-registered from edg-backend's report 12, section 348, before any run: rejected (observed on the two probe files separately, not predicted; the combined file has not been probed)
#include <cerastes.h>
#include <stddef.h>
#include <stdlib.h>


struct U {
  int n;
  int m[2][3];
};

struct R {
  struct U *q;
  int z;
};

struct OwnBeside {
  int m[2][3];  // <- here
  int *p CERASTES_OWNING;
};

struct MarkBeside {
  int m[2][3];  // <- here
  int *p CERASTES_REF_TO_UNINIT;
};

struct OwnT {
  CERASTES_OWNING struct U *q;  // <- here
  int z;
};

int f_malloc(void)
{
  struct U *p = (struct U *)malloc(sizeof(struct U));  // <- here
  if (p == NULL) {
    return 0;
  }
  free(p);  // <- here
  return 1;
}

int f_calloc(void)
{
  struct U *p = (struct U *)calloc(1, sizeof(struct U));  // <- here
  if (p == NULL) {
    return 0;
  }
  free(p);  // <- here
  return 1;
}

void f_free(struct U *p)
    CERASTES_REQUIRES(cerastes_owns_block(p))  // <- here
{
  free(p);  // <- here
}

void f_borrow(struct U *p)
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

void f_ref(struct U *p CERASTES_REF_TO_UNINIT)  // <- here
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  (void)p;
}

void f_must(struct U *p CERASTES_MUST_INIT)  // <- here
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  (void)p;
}

void f_owning(CERASTES_OWNING struct U *p)  // <- here
{
  (void)p;
}

CERASTES_OWNING struct U *f_result(void);  // <- here

struct U *f_fresh(void)
    CERASTES_ENSURES(r, cerastes_owns(*r));  // <- here

int through_load_atom(struct R *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(*p->q))  // <- here
{
  return p->z;
}

int through_load_read(struct R *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && p->q->m[1][2] > 0)  // <- here
{
  return p->z;
}

void loop_atom(struct U *p)
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

void give(struct U *p)
    CERASTES_ENSURES(cerastes_owns(p->m));  // <- here

int call_fresh(void)
{
  struct U *p = f_fresh();
  return p == NULL;
}

CERASTES_OWNING static struct U *gp = NULL;  // <- here

int owning_local(void)
{
  CERASTES_OWNING struct U *l = NULL;  // <- here
  return l == NULL;
}

void loop_member(struct U *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 2 && cerastes_owns(p->m))  // <- here
  CERASTES_DECREASES(2 - i)
  while (i < 2) {
    i = i + 1;
  }
}

void loop_reads(struct U *p)
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

void give_whole(struct U *p)
    CERASTES_ENSURES(cerastes_owns(*p));  // <- here

int use_gp(void)
{
  return gp == NULL;
}

// expect: rejected cerastes-unsupported_annotation @ L20
// expect: rejected cerastes-unsupported_annotation @ L30
// mode: c23
// c-only: restrict does not exist in C++20 (EDG, clang++ and g++ refuse it, decision 0037 item 2)
// twin: tests/memory/restrict_body_owns_global.c
// Pins: 0064 item 19 (question 326; audit round 2, issue 6), the clauses with no pin of their own: (i) a global that reaches the pointee's field only through a pointer, int *gp, whose ADDRESS the body takes (the body names gp without a contract atom: reading it would need one); (iii) a callee whose parameter points to a structure that holds a pointer (struct H { int *q; }). Each is rejected at its function head. Clause (ii) for an atom in a callee's ensures ONLY is not expressible: the contract rule "an `ensures` owns with no owns of the same global in the `requires`, in a contract with no body in this unit" rejects the callee's declaration first, so there is no row for it here (the first draft's row, a giver `give_g`, printed that rejection at the declaration and nothing at the caller's head: question 160, report 14); (ii) with the atom in both contracts is restrict_body_callee_global.c
// Pre-registered from record 0064 item 19 as revised by audit round 2 (issue 6), before any run: rejected
#include <cerastes.h>

int *gp;

struct H {
  int *q;
};

void takes(struct H *h)
    CERASTES_REQUIRES(cerastes_owns(*h))
    CERASTES_ENSURES(cerastes_owns(*h));

int pointer_global(int *restrict p)  // <- here
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(r, cerastes_owns(*p) && r == 1)
{
  int **pp = &gp;
  (void)pp;
  *p = 1;
  return *p;
}

int structure_with_pointer(int *restrict p)  // <- here
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(r, cerastes_owns(*p) && r == 1)
{
  struct H hv = {0};
  *p = 1;
  takes(&hv);
  return *p;
}

// expect: rejected cerastes-unsupported_annotation @ L12
// expect: rejected cerastes-unsupported_annotation @ L21
// mode: c23
// c-only: restrict does not exist in C++20 (EDG, clang++ and g++ refuse it, decision 0037 item 2)
// twin: tests/memory/restrict_dropped.c
// Pins: 0064 item 19 (question 326, the owner): every restrict parameter is rejected, unsupported_annotation, once per function at its head; drop the keyword to verify the function (tests/memory/restrict_dropped.c). Re-expected from verified by Flips R. It verified before as: 0064 item 19 (question 326): a function whose own contract holds the global whole and unguarded, by a written owns(g) or reads(g) beside owns(*p), may name it: the permissions exclude the alias p == &g (the probes d1 and d2: the alias branch is dead), so the body writes g and *p (own_g) or reads g (read_g) and the model is C's; struct_global_restrict.c is the same rule over a structure global
// Pre-registered from record 0064 item 19 (Part R, Flips R), before the run: one row per function with a restrict parameter, at its head line
#include <cerastes.h>

int g;

int own_g(int *restrict p)  // <- here
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(g))
    CERASTES_ENSURES(r, cerastes_owns(*p) && cerastes_owns(g) && g == 2 && r == 1)
{
  *p = 1;
  g = 2;
  return *p;
}

int read_g(int *restrict p)  // <- here
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_reads(g) && 0 <= g && g < 100)
    CERASTES_ENSURES(r, cerastes_owns(*p) && cerastes_reads(g) && r == 1 + g)
{
  *p = 1;
  return *p + g;
}

int main(void)
{
  int a = 0;
  int x = own_g(&a);
  int y = read_g(&a);
  return x + y - 4;
}

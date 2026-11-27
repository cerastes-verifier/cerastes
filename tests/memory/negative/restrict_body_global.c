// expect: rejected cerastes-unsupported_annotation @ L27
// expect: rejected cerastes-unsupported_annotation @ L39
// expect: rejected cerastes-unsupported_annotation @ L51
// expect: rejected cerastes-unsupported_annotation @ L63
// expect: rejected cerastes-unsupported_annotation @ L76
// expect: rejected cerastes-unsupported_annotation @ L89
// expect: rejected cerastes-unsupported_annotation @ L98
// expect: rejected cerastes-unsupported_annotation @ L110
// mode: c23
// c-only: restrict does not exist in C++20 (EDG, clang++ and g++ refuse it, decision 0037 item 2)
// twin: tests/memory/restrict_dropped.c
// Pins: 0064 item 19 (question 326), clause (i): a restrict parameter in a function whose body names, or takes the address of, a global or static local that holds the pointee's field, unless the contract holds it whole and unguarded: a global by name (probe x07, c1a), an element of a global array (x05), a member of a structure global (x09's kind), a local int *q = &g (x08, c1b), a local structure holding &g (c1d), the read-only body (c6c: over-strict by design, C allows it), a partial atom owns(g.x) with p == &g.y tested in the body (d4b), and a guarded atom (c == 0 || owns(g)) (d5: over-strict, left out of the exemption). Each is rejected at its function head. Every alias branch of the first five and of d4b is live on the base (the twins with the other value fail) [0064 item 19, question 326: every restrict parameter is now rejected by that one rule, whatever the contract; this pin's row keeps its tag and line and its condition no longer matters]
// Pre-registered from record 0064 item 19 (the base verifies each of them: reports 4, rounds 1 and 2), before any run: rejected
#include <cerastes.h>

int g;
int g4[4];
struct G {
  int x;
  int y;
};
struct G gs;
struct S {
  int *q;
};

int by_name(int *restrict p)  // <- here
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(r, cerastes_owns(*p) && r == 2)
{
  if (p == &g) {
    *p = 1;
    g = 2;
    return *p;
  }
  return 2;
}

int array_element(int *restrict p)  // <- here
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(r, cerastes_owns(*p) && r == 2)
{
  if (p == &g4[1]) {
    *p = 1;
    g4[1] = 2;
    return *p;
  }
  return 2;
}

int member(int *restrict p)  // <- here
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(r, cerastes_owns(*p) && r == 2)
{
  if (p == &gs.y) {
    *p = 1;
    gs.y = 2;
    return *p;
  }
  return 2;
}

int local_pointer(int *restrict p)  // <- here
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(r, cerastes_owns(*p) && r == 2)
{
  int *q = &g;
  if (p == q) {
    *p = 1;
    *q = 2;
    return *p;
  }
  return 2;
}

int local_structure(int *restrict p)  // <- here
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(r, cerastes_owns(*p) && r == 2)
{
  struct S s = {&g};
  if (p == s.q) {
    *p = 1;
    *s.q = 2;
    return *p;
  }
  return 2;
}

int read_only(int *restrict p)  // <- here
    CERASTES_REQUIRES(cerastes_owns(*p) && p == &g)
    CERASTES_ENSURES(r, cerastes_owns(*p) && r == 0)
{
  int a = *p;
  int b = g;
  return a - b;
}

int partial_atom(int *restrict p)  // <- here
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(gs.x))
    CERASTES_ENSURES(r, cerastes_owns(*p) && cerastes_owns(gs.x) && r == 2)
{
  if (p == &gs.y) {
    *p = 1;
    gs.y = 2;
    return *p;
  }
  return 2;
}

int guarded_atom(int *restrict p, int c)  // <- here
    CERASTES_REQUIRES(cerastes_owns(*p) && c != 0 && (c == 0 || cerastes_owns(g)))
    CERASTES_ENSURES(r, cerastes_owns(*p) && r == 2)
{
  if (p == &g) {
    *p = 1;
    g = 2;
    return *p;
  }
  return 2;
}

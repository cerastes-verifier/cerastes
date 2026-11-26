// expect: rejected cerastes-unsupported_annotation @ L28
// expect: rejected cerastes-unsupported_annotation @ L36
// expect: rejected cerastes-unsupported_annotation @ L43
// mode: c23
// c-only: restrict does not exist in C++20 (EDG, clang++ and g++ refuse it, decision 0037 item 2)
// twin: tests/memory/restrict_dropped.c
// Pins: 0064 item 19 (question 332, 338): a restrict parameter on a declaration without a body is rejected where the encoder first reads its signature: at the FIRST call of it in the unit (put, called once; twice, called from two functions: one row, at the first call), or at the declaration itself when the signature is read early whatever the calls (put_struct: a pointer-to-structure parameter, never called). Two forms stay accepted and carry no row: unused, an uncalled bodiless restrict prototype beside verified functions (neither verified nor called, so no result rests on it), and keep, a restrict prototype before a definition without the keyword (the qualifier is read from the definition, C 6.7.3.1)
// Pre-registered from record 0064 item 19 as settled for question 338 (edg-backend's report 3), before the run: rows at put's call, twice's first call and put_struct's declaration; none for unused and keep
#include <cerastes.h>
#include <stddef.h>

struct S {
  int x;
};

void put(int *restrict p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p) && *p == 5);

void twice(int *restrict p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p));

void unused(int *restrict p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p));

void put_struct(struct S *restrict p)  // <- here
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p));

int use(void)
    CERASTES_ENSURES(r, r == 5)
{
  int x = 1;
  put(&x);  // <- here
  return x;
}

void first(void)
{
  int x = 1;
  twice(&x);  // <- here
}

void second(void)
{
  int x = 1;
  twice(&x);
}

int keep(int *restrict p);

int keep(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(r, cerastes_owns(*p) && r == *p)
{
  return *p;
}

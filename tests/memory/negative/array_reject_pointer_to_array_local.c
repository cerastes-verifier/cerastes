// expect: rejected cerastes-unsupported_type @ L12
// expect: rejected cerastes-unsupported_type @ L20
// mode: c23, c++20
// twin: tests/memory/array_rows_spellings.c
// Pins: 0064 items 1 and 13 (question 320: the pointer-to-array parameter is accepted, the new pin of this one names the rest; M8): a pointer to an array as a local (int (*p)[3] = m;) and the address of a row (&m[i]) stay rejected: a pointer to an array is a type of a parameter only
// Pre-registered from record 0064 items 1 and 13 (Part E), before any run: rejected, unsupported_type at both lines (the frozen build's probes of the one-dimensional analogs int (*p)[3] = &a; and (void)&a; print unsupported_type at those lines; the use of p, a second row there, is left out)
#include <cerastes.h>

int local(void)
{
  int m[2][3] = {{1, 2, 3}, {4, 5, 6}};
  [[maybe_unused]] int (*p)[3] = m;  // <- here
  return m[1][2];
}

int address(int i)
    CERASTES_REQUIRES(0 <= i && i < 2)
{
  int m[2][3] = {{1, 2, 3}, {4, 5, 6}};
  (void)&m[i];  // <- here
  return m[i][0];
}

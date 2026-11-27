// expect: rejected cerastes-unsupported_type @ L12
// expect: rejected cerastes-unsupported_type @ L17
// expect: rejected cerastes-unsupported_type @ L22
// mode: c23, c++20
// twin: tests/memory/array_rows_spellings.c
// Pins: 0064 item 13 (question 352, settled with the advisor on 2026-11-26; M4.6): a const row parameter stays rejected in each spelling, const int a[][3], const int (*a)[3] and const row *a (a qualified row reaches the encoder as a fresh array of const int with no typeref above it): item 2 tests the const rule first, so it is the const-array rejection at the function's line, stricter than one dimension, where const int *p is accepted
// Pre-registered from record 0064 items 2 and 13 (Part E), before any run: rejected (tag by array_reject_const_array.c, unsupported_type, at each function's line; to confirm at the first run)
#include <cerastes.h>

typedef int row[3];

int plain(const int a[][3])  // <- here
{
  return a[0][0] * 0;
}

int star(const int (*a)[3])  // <- here
{
  return a[0][0] * 0;
}

int typedefed(const row *a)  // <- here
{
  return a[0][0] * 0;
}

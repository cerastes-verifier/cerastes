// expect: rejected cerastes-array_address_null_test @ L56
// expect: rejected cerastes-array_address_null_test @ L68
// expect: rejected cerastes-array_address_null_test @ L78
// expect: rejected cerastes-array_address_null_test @ L88
// expect: rejected cerastes-array_address_null_test @ L98
// expect: rejected cerastes-array_address_null_test @ L108
// expect: rejected cerastes-array_address_null_test @ L118
// expect: rejected cerastes-array_address_null_test @ L128
// expect: rejected cerastes-array_address_null_test @ L138
// expect: rejected cerastes-array_address_null_test @ L148
// expect: rejected cerastes-array_address_null_test @ L160
// expect: rejected cerastes-array_address_null_test @ L172
// expect: rejected cerastes-array_address_null_test @ L179
// expect: rejected cerastes-array_address_null_test @ L190
// expect: rejected cerastes-array_address_null_test @ L204
// expect: rejected cerastes-array_address_null_test @ L218
// expect: rejected cerastes-array_address_null_test @ L230
// expect: rejected cerastes-array_address_null_test @ L240
// expect: rejected cerastes-array_address_null_test @ L250
// expect: rejected cerastes-array_address_null_test @ L260
// expect: rejected cerastes-array_address_null_test @ L270
// expect: rejected cerastes-array_address_null_test @ L280
// expect: rejected cerastes-array_address_null_test @ L290
// expect: rejected cerastes-array_address_null_test @ L300
// expect: rejected cerastes-array_address_null_test @ L310
// expect: rejected cerastes-array_address_null_test @ L322
// expect: rejected cerastes-array_address_null_test @ L334
// expect: rejected cerastes-array_address_null_test @ L341
// expect: rejected cerastes-unsupported_expression @ L341
// expect: rejected cerastes-array_address_null_test @ L352
// expect: rejected cerastes-unsupported_expression @ L352
// expect: rejected cerastes-array_address_null_test @ L366
// mode: c23, c++20
// twin: tests/memory/matrix_global.c
// Pins: 0064 item 5 (question 322, audit round E1: every truth test of a row or an element address is array_address_null_test, as a != 0 of a one-dimensional array is), a GLOBAL matrix int g[2][3] at non-constant indexes: the row g[i] and &g[i][j], the contract forms beside: one function per form (if, while, !, &&, ||, ?:, (bool), (_Bool), assert, CERASTES_CHECK, for, do-while, a loop invariant)
// Pre-registered from edg-backend's report 12 ("ROW TRUTH TESTS", the sweep's probes on edg27 over this file's body, both modes), before the first run: rejected (observed, not predicted); every marked line is one function
#include <assert.h>
#include <cerastes.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
#define BOOL bool
#else
#define BOOL _Bool
#endif

int g[2][3];

int f_if_row(int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns(g) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns(g))
{
  int r = 0;
  int k = 0;
  if (g[i]) { r = 1; }  // <- here
  return r + k;
}

int f_while_row(int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns(g) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns(g))
{
  int r = 0;
  int k = 0;
  CERASTES_INVARIANT(0 <= k && k <= 2)
  CERASTES_DECREASES(2 - k)
  while (g[i]) { k = k + 1; }  // <- here
  return r + k;
}

int f_not_row(int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns(g) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns(g))
{
  int r = 0;
  int k = 0;
  r = !g[i];  // <- here
  return r + k;
}

int f_and_row(int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns(g) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns(g))
{
  int r = 0;
  int k = 0;
  r = g[i] && c;  // <- here
  return r + k;
}

int f_or_row(int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns(g) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns(g))
{
  int r = 0;
  int k = 0;
  r = c || g[i];  // <- here
  return r + k;
}

int f_cond_row(int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns(g) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns(g))
{
  int r = 0;
  int k = 0;
  r = g[i] ? 1 : 2;  // <- here
  return r + k;
}

int f_cast_bool_row(int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns(g) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns(g))
{
  int r = 0;
  int k = 0;
  r = (bool)g[i];  // <- here
  return r + k;
}

int f_cast_Bool_row(int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns(g) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns(g))
{
  int r = 0;
  int k = 0;
  r = (BOOL)g[i];  // <- here
  return r + k;
}

int f_assert_row(int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns(g) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns(g))
{
  int r = 0;
  int k = 0;
  assert(g[i]);  // <- here
  return r + k;
}

int f_check_row(int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns(g) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns(g))
{
  int r = 0;
  int k = 0;
  CERASTES_CHECK(g[i]);  // <- here
  return r + k;
}

int f_for_row(int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns(g) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns(g))
{
  int r = 0;
  int k = 0;
  CERASTES_INVARIANT(0 <= k && k <= 2)
  CERASTES_DECREASES(2 - k)
  for (; g[i]; ) { k = k + 1; }  // <- here
  return r + k;
}

int f_dowhile_row(int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns(g) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns(g))
{
  int r = 0;
  int k = 0;
  CERASTES_INVARIANT(0 <= k && k <= 2)
  CERASTES_DECREASES(2 - k)
  do { k = k + 1; } while (g[i]);  // <- here
  return r + k;
}

int f_c_requires_row(int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns(g) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns(g))
    CERASTES_REQUIRES(g[i])  // <- here
{
  (void)i;
  (void)j;
  (void)c;
  return 0;
}

int f_c_ensures_row(int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns(g) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns(g))
    CERASTES_ENSURES(r, r == 0 && g[i])  // <- here
{
  (void)i;
  (void)j;
  (void)c;
  return 0;
}

int f_invariant_row(int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns(g) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns(g))
{
  int r = 0;
  int k = 0;
  CERASTES_INVARIANT(0 <= k && k <= 2 && g[i])  // <- here
  CERASTES_DECREASES(2 - k)
  while (k < 2) {
    k = k + 1;
  }
  return r + k;
}

int f_if_elem(int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns(g) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns(g))
{
  int r = 0;
  int k = 0;
  if (&g[i][j]) { r = 1; }  // <- here
  return r + k;
}

int f_while_elem(int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns(g) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns(g))
{
  int r = 0;
  int k = 0;
  CERASTES_INVARIANT(0 <= k && k <= 2)
  CERASTES_DECREASES(2 - k)
  while (&g[i][j]) { k = k + 1; }  // <- here
  return r + k;
}

int f_not_elem(int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns(g) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns(g))
{
  int r = 0;
  int k = 0;
  r = !&g[i][j];  // <- here
  return r + k;
}

int f_and_elem(int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns(g) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns(g))
{
  int r = 0;
  int k = 0;
  r = &g[i][j] && c;  // <- here
  return r + k;
}

int f_or_elem(int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns(g) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns(g))
{
  int r = 0;
  int k = 0;
  r = c || &g[i][j];  // <- here
  return r + k;
}

int f_cond_elem(int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns(g) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns(g))
{
  int r = 0;
  int k = 0;
  r = &g[i][j] ? 1 : 2;  // <- here
  return r + k;
}

int f_cast_bool_elem(int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns(g) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns(g))
{
  int r = 0;
  int k = 0;
  r = (bool)&g[i][j];  // <- here
  return r + k;
}

int f_cast_Bool_elem(int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns(g) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns(g))
{
  int r = 0;
  int k = 0;
  r = (BOOL)&g[i][j];  // <- here
  return r + k;
}

int f_assert_elem(int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns(g) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns(g))
{
  int r = 0;
  int k = 0;
  assert(&g[i][j]);  // <- here
  return r + k;
}

int f_check_elem(int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns(g) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns(g))
{
  int r = 0;
  int k = 0;
  CERASTES_CHECK(&g[i][j]);  // <- here
  return r + k;
}

int f_for_elem(int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns(g) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns(g))
{
  int r = 0;
  int k = 0;
  CERASTES_INVARIANT(0 <= k && k <= 2)
  CERASTES_DECREASES(2 - k)
  for (; &g[i][j]; ) { k = k + 1; }  // <- here
  return r + k;
}

int f_dowhile_elem(int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns(g) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns(g))
{
  int r = 0;
  int k = 0;
  CERASTES_INVARIANT(0 <= k && k <= 2)
  CERASTES_DECREASES(2 - k)
  do { k = k + 1; } while (&g[i][j]);  // <- here
  return r + k;
}

int f_c_requires_elem(int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns(g) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns(g))
    CERASTES_REQUIRES(&g[i][j])  // <- here
{
  (void)i;
  (void)j;
  (void)c;
  return 0;
}

int f_c_ensures_elem(int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns(g) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns(g))
    CERASTES_ENSURES(r, r == 0 && &g[i][j])  // <- here
{
  (void)i;
  (void)j;
  (void)c;
  return 0;
}

int f_invariant_elem(int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns(g) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns(g))
{
  int r = 0;
  int k = 0;
  CERASTES_INVARIANT(0 <= k && k <= 2 && &g[i][j])  // <- here
  CERASTES_DECREASES(2 - k)
  while (k < 2) {
    k = k + 1;
  }
  return r + k;
}

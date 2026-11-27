// expect: rejected cerastes-array_address_null_test @ L55
// expect: rejected cerastes-array_address_null_test @ L67
// expect: rejected cerastes-array_address_null_test @ L77
// expect: rejected cerastes-array_address_null_test @ L87
// expect: rejected cerastes-array_address_null_test @ L97
// expect: rejected cerastes-array_address_null_test @ L107
// expect: rejected cerastes-array_address_null_test @ L117
// expect: rejected cerastes-array_address_null_test @ L127
// expect: rejected cerastes-array_address_null_test @ L137
// expect: rejected cerastes-array_address_null_test @ L147
// expect: rejected cerastes-array_address_null_test @ L159
// expect: rejected cerastes-array_address_null_test @ L171
// expect: rejected cerastes-array_address_null_test @ L178
// expect: rejected cerastes-array_address_null_test @ L189
// expect: rejected cerastes-array_address_null_test @ L203
// expect: rejected cerastes-array_address_null_test @ L217
// expect: rejected cerastes-array_address_null_test @ L229
// expect: rejected cerastes-array_address_null_test @ L239
// expect: rejected cerastes-array_address_null_test @ L249
// expect: rejected cerastes-array_address_null_test @ L259
// expect: rejected cerastes-array_address_null_test @ L269
// expect: rejected cerastes-array_address_null_test @ L279
// expect: rejected cerastes-array_address_null_test @ L289
// expect: rejected cerastes-array_address_null_test @ L299
// expect: rejected cerastes-array_address_null_test @ L309
// expect: rejected cerastes-array_address_null_test @ L321
// expect: rejected cerastes-array_address_null_test @ L333
// expect: rejected cerastes-array_address_null_test @ L340
// expect: rejected cerastes-unsupported_expression @ L340
// expect: rejected cerastes-array_address_null_test @ L351
// expect: rejected cerastes-unsupported_expression @ L351
// expect: rejected cerastes-array_address_null_test @ L365
// mode: c23, c++20
// twin: tests/memory/array_rows_param.c
// Pins: 0064 item 5 (question 322, audit round E1: every truth test of a row or an element address is array_address_null_test, as a != 0 of a one-dimensional array is), a ROW PARAMETER int a[][3]: a[i] and &a[i][j], the contract forms beside: one function per form (if, while, !, &&, ||, ?:, (bool), (_Bool), assert, CERASTES_CHECK, for, do-while, a loop invariant)
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


int f_if_row(int a[][3], int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns_range(a, 2) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns_range(a, 2))
{
  int r = 0;
  int k = 0;
  if (a[i]) { r = 1; }  // <- here
  return r + k;
}

int f_while_row(int a[][3], int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns_range(a, 2) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns_range(a, 2))
{
  int r = 0;
  int k = 0;
  CERASTES_INVARIANT(0 <= k && k <= 2)
  CERASTES_DECREASES(2 - k)
  while (a[i]) { k = k + 1; }  // <- here
  return r + k;
}

int f_not_row(int a[][3], int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns_range(a, 2) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns_range(a, 2))
{
  int r = 0;
  int k = 0;
  r = !a[i];  // <- here
  return r + k;
}

int f_and_row(int a[][3], int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns_range(a, 2) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns_range(a, 2))
{
  int r = 0;
  int k = 0;
  r = a[i] && c;  // <- here
  return r + k;
}

int f_or_row(int a[][3], int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns_range(a, 2) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns_range(a, 2))
{
  int r = 0;
  int k = 0;
  r = c || a[i];  // <- here
  return r + k;
}

int f_cond_row(int a[][3], int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns_range(a, 2) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns_range(a, 2))
{
  int r = 0;
  int k = 0;
  r = a[i] ? 1 : 2;  // <- here
  return r + k;
}

int f_cast_bool_row(int a[][3], int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns_range(a, 2) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns_range(a, 2))
{
  int r = 0;
  int k = 0;
  r = (bool)a[i];  // <- here
  return r + k;
}

int f_cast_Bool_row(int a[][3], int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns_range(a, 2) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns_range(a, 2))
{
  int r = 0;
  int k = 0;
  r = (BOOL)a[i];  // <- here
  return r + k;
}

int f_assert_row(int a[][3], int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns_range(a, 2) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns_range(a, 2))
{
  int r = 0;
  int k = 0;
  assert(a[i]);  // <- here
  return r + k;
}

int f_check_row(int a[][3], int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns_range(a, 2) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns_range(a, 2))
{
  int r = 0;
  int k = 0;
  CERASTES_CHECK(a[i]);  // <- here
  return r + k;
}

int f_for_row(int a[][3], int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns_range(a, 2) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns_range(a, 2))
{
  int r = 0;
  int k = 0;
  CERASTES_INVARIANT(0 <= k && k <= 2)
  CERASTES_DECREASES(2 - k)
  for (; a[i]; ) { k = k + 1; }  // <- here
  return r + k;
}

int f_dowhile_row(int a[][3], int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns_range(a, 2) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns_range(a, 2))
{
  int r = 0;
  int k = 0;
  CERASTES_INVARIANT(0 <= k && k <= 2)
  CERASTES_DECREASES(2 - k)
  do { k = k + 1; } while (a[i]);  // <- here
  return r + k;
}

int f_c_requires_row(int a[][3], int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns_range(a, 2) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns_range(a, 2))
    CERASTES_REQUIRES(a[i])  // <- here
{
  (void)i;
  (void)j;
  (void)c;
  return 0;
}

int f_c_ensures_row(int a[][3], int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns_range(a, 2) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns_range(a, 2))
    CERASTES_ENSURES(r, r == 0 && a[i])  // <- here
{
  (void)i;
  (void)j;
  (void)c;
  return 0;
}

int f_invariant_row(int a[][3], int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns_range(a, 2) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns_range(a, 2))
{
  int r = 0;
  int k = 0;
  CERASTES_INVARIANT(0 <= k && k <= 2 && a[i])  // <- here
  CERASTES_DECREASES(2 - k)
  while (k < 2) {
    k = k + 1;
  }
  return r + k;
}

int f_if_elem(int a[][3], int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns_range(a, 2) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns_range(a, 2))
{
  int r = 0;
  int k = 0;
  if (&a[i][j]) { r = 1; }  // <- here
  return r + k;
}

int f_while_elem(int a[][3], int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns_range(a, 2) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns_range(a, 2))
{
  int r = 0;
  int k = 0;
  CERASTES_INVARIANT(0 <= k && k <= 2)
  CERASTES_DECREASES(2 - k)
  while (&a[i][j]) { k = k + 1; }  // <- here
  return r + k;
}

int f_not_elem(int a[][3], int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns_range(a, 2) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns_range(a, 2))
{
  int r = 0;
  int k = 0;
  r = !&a[i][j];  // <- here
  return r + k;
}

int f_and_elem(int a[][3], int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns_range(a, 2) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns_range(a, 2))
{
  int r = 0;
  int k = 0;
  r = &a[i][j] && c;  // <- here
  return r + k;
}

int f_or_elem(int a[][3], int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns_range(a, 2) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns_range(a, 2))
{
  int r = 0;
  int k = 0;
  r = c || &a[i][j];  // <- here
  return r + k;
}

int f_cond_elem(int a[][3], int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns_range(a, 2) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns_range(a, 2))
{
  int r = 0;
  int k = 0;
  r = &a[i][j] ? 1 : 2;  // <- here
  return r + k;
}

int f_cast_bool_elem(int a[][3], int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns_range(a, 2) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns_range(a, 2))
{
  int r = 0;
  int k = 0;
  r = (bool)&a[i][j];  // <- here
  return r + k;
}

int f_cast_Bool_elem(int a[][3], int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns_range(a, 2) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns_range(a, 2))
{
  int r = 0;
  int k = 0;
  r = (BOOL)&a[i][j];  // <- here
  return r + k;
}

int f_assert_elem(int a[][3], int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns_range(a, 2) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns_range(a, 2))
{
  int r = 0;
  int k = 0;
  assert(&a[i][j]);  // <- here
  return r + k;
}

int f_check_elem(int a[][3], int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns_range(a, 2) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns_range(a, 2))
{
  int r = 0;
  int k = 0;
  CERASTES_CHECK(&a[i][j]);  // <- here
  return r + k;
}

int f_for_elem(int a[][3], int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns_range(a, 2) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns_range(a, 2))
{
  int r = 0;
  int k = 0;
  CERASTES_INVARIANT(0 <= k && k <= 2)
  CERASTES_DECREASES(2 - k)
  for (; &a[i][j]; ) { k = k + 1; }  // <- here
  return r + k;
}

int f_dowhile_elem(int a[][3], int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns_range(a, 2) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns_range(a, 2))
{
  int r = 0;
  int k = 0;
  CERASTES_INVARIANT(0 <= k && k <= 2)
  CERASTES_DECREASES(2 - k)
  do { k = k + 1; } while (&a[i][j]);  // <- here
  return r + k;
}

int f_c_requires_elem(int a[][3], int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns_range(a, 2) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns_range(a, 2))
    CERASTES_REQUIRES(&a[i][j])  // <- here
{
  (void)i;
  (void)j;
  (void)c;
  return 0;
}

int f_c_ensures_elem(int a[][3], int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns_range(a, 2) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns_range(a, 2))
    CERASTES_ENSURES(r, r == 0 && &a[i][j])  // <- here
{
  (void)i;
  (void)j;
  (void)c;
  return 0;
}

int f_invariant_elem(int a[][3], int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns_range(a, 2) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns_range(a, 2))
{
  int r = 0;
  int k = 0;
  CERASTES_INVARIANT(0 <= k && k <= 2 && &a[i][j])  // <- here
  CERASTES_DECREASES(2 - k)
  while (k < 2) {
    k = k + 1;
  }
  return r + k;
}

// expect: rejected cerastes-array_address_null_test @ L53
// expect: rejected cerastes-array_address_null_test @ L65
// expect: rejected cerastes-array_address_null_test @ L75
// expect: rejected cerastes-array_address_null_test @ L85
// expect: rejected cerastes-array_address_null_test @ L95
// expect: rejected cerastes-array_address_null_test @ L105
// expect: rejected cerastes-array_address_null_test @ L115
// expect: rejected cerastes-array_address_null_test @ L125
// expect: rejected cerastes-array_address_null_test @ L135
// expect: rejected cerastes-array_address_null_test @ L145
// expect: rejected cerastes-array_address_null_test @ L157
// expect: rejected cerastes-array_address_null_test @ L169
// expect: rejected cerastes-array_address_null_test @ L179
// expect: rejected cerastes-array_address_null_test @ L193
// expect: rejected cerastes-array_address_null_test @ L205
// expect: rejected cerastes-array_address_null_test @ L215
// expect: rejected cerastes-array_address_null_test @ L225
// expect: rejected cerastes-array_address_null_test @ L235
// expect: rejected cerastes-array_address_null_test @ L245
// expect: rejected cerastes-array_address_null_test @ L255
// expect: rejected cerastes-array_address_null_test @ L265
// expect: rejected cerastes-array_address_null_test @ L275
// expect: rejected cerastes-array_address_null_test @ L285
// expect: rejected cerastes-array_address_null_test @ L297
// expect: rejected cerastes-array_address_null_test @ L309
// expect: rejected cerastes-array_address_null_test @ L319
// expect: frontend-warning set_but_not_used @ L142
// expect: frontend-warning set_but_not_used @ L176
// expect: frontend-warning set_but_not_used @ L282
// expect: frontend-warning set_but_not_used @ L316
// mode: c23, c++20
// twin: tests/memory/matrix_address.c
// Pins: 0064 item 5 (question 322, audit round E1: every truth test of a row or an element address is array_address_null_test, as a != 0 of a one-dimensional array is), a `static` local matrix int sm[2][3]: sm[i] and &sm[i][j]: one function per form (if, while, !, &&, ||, ?:, (bool), (_Bool), assert, CERASTES_CHECK, for, do-while, a loop invariant)
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


int f_if_row(int i, int j, int c)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 3)
{
  static int sm[2][3];
  int r = 0;
  int k = 0;
  if (sm[i]) { r = 1; }  // <- here
  return r + k;
}

int f_while_row(int i, int j, int c)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 3)
{
  static int sm[2][3];
  int r = 0;
  int k = 0;
  CERASTES_INVARIANT(0 <= k && k <= 2)
  CERASTES_DECREASES(2 - k)
  while (sm[i]) { k = k + 1; }  // <- here
  return r + k;
}

int f_not_row(int i, int j, int c)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 3)
{
  static int sm[2][3];
  int r = 0;
  int k = 0;
  r = !sm[i];  // <- here
  return r + k;
}

int f_and_row(int i, int j, int c)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 3)
{
  static int sm[2][3];
  int r = 0;
  int k = 0;
  r = sm[i] && c;  // <- here
  return r + k;
}

int f_or_row(int i, int j, int c)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 3)
{
  static int sm[2][3];
  int r = 0;
  int k = 0;
  r = c || sm[i];  // <- here
  return r + k;
}

int f_cond_row(int i, int j, int c)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 3)
{
  static int sm[2][3];
  int r = 0;
  int k = 0;
  r = sm[i] ? 1 : 2;  // <- here
  return r + k;
}

int f_cast_bool_row(int i, int j, int c)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 3)
{
  static int sm[2][3];
  int r = 0;
  int k = 0;
  r = (bool)sm[i];  // <- here
  return r + k;
}

int f_cast_Bool_row(int i, int j, int c)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 3)
{
  static int sm[2][3];
  int r = 0;
  int k = 0;
  r = (BOOL)sm[i];  // <- here
  return r + k;
}

int f_assert_row(int i, int j, int c)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 3)
{
  static int sm[2][3];
  int r = 0;
  int k = 0;
  assert(sm[i]);  // <- here
  return r + k;
}

int f_check_row(int i, int j, int c)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 3)
{
  static int sm[2][3];
  int r = 0;
  int k = 0;
  CERASTES_CHECK(sm[i]);  // <- here
  return r + k;
}

int f_for_row(int i, int j, int c)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 3)
{
  static int sm[2][3];
  int r = 0;
  int k = 0;
  CERASTES_INVARIANT(0 <= k && k <= 2)
  CERASTES_DECREASES(2 - k)
  for (; sm[i]; ) { k = k + 1; }  // <- here
  return r + k;
}

int f_dowhile_row(int i, int j, int c)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 3)
{
  static int sm[2][3];
  int r = 0;
  int k = 0;
  CERASTES_INVARIANT(0 <= k && k <= 2)
  CERASTES_DECREASES(2 - k)
  do { k = k + 1; } while (sm[i]);  // <- here
  return r + k;
}

int f_invariant_row(int i, int j, int c)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 3)
{
  static int sm[2][3];
  int r = 0;
  int k = 0;
  CERASTES_INVARIANT(0 <= k && k <= 2 && sm[i])  // <- here
  CERASTES_DECREASES(2 - k)
  while (k < 2) {
    k = k + 1;
  }
  return r + k;
}

int f_if_elem(int i, int j, int c)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 3)
{
  static int sm[2][3];
  int r = 0;
  int k = 0;
  if (&sm[i][j]) { r = 1; }  // <- here
  return r + k;
}

int f_while_elem(int i, int j, int c)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 3)
{
  static int sm[2][3];
  int r = 0;
  int k = 0;
  CERASTES_INVARIANT(0 <= k && k <= 2)
  CERASTES_DECREASES(2 - k)
  while (&sm[i][j]) { k = k + 1; }  // <- here
  return r + k;
}

int f_not_elem(int i, int j, int c)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 3)
{
  static int sm[2][3];
  int r = 0;
  int k = 0;
  r = !&sm[i][j];  // <- here
  return r + k;
}

int f_and_elem(int i, int j, int c)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 3)
{
  static int sm[2][3];
  int r = 0;
  int k = 0;
  r = &sm[i][j] && c;  // <- here
  return r + k;
}

int f_or_elem(int i, int j, int c)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 3)
{
  static int sm[2][3];
  int r = 0;
  int k = 0;
  r = c || &sm[i][j];  // <- here
  return r + k;
}

int f_cond_elem(int i, int j, int c)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 3)
{
  static int sm[2][3];
  int r = 0;
  int k = 0;
  r = &sm[i][j] ? 1 : 2;  // <- here
  return r + k;
}

int f_cast_bool_elem(int i, int j, int c)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 3)
{
  static int sm[2][3];
  int r = 0;
  int k = 0;
  r = (bool)&sm[i][j];  // <- here
  return r + k;
}

int f_cast_Bool_elem(int i, int j, int c)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 3)
{
  static int sm[2][3];
  int r = 0;
  int k = 0;
  r = (BOOL)&sm[i][j];  // <- here
  return r + k;
}

int f_assert_elem(int i, int j, int c)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 3)
{
  static int sm[2][3];
  int r = 0;
  int k = 0;
  assert(&sm[i][j]);  // <- here
  return r + k;
}

int f_check_elem(int i, int j, int c)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 3)
{
  static int sm[2][3];
  int r = 0;
  int k = 0;
  CERASTES_CHECK(&sm[i][j]);  // <- here
  return r + k;
}

int f_for_elem(int i, int j, int c)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 3)
{
  static int sm[2][3];
  int r = 0;
  int k = 0;
  CERASTES_INVARIANT(0 <= k && k <= 2)
  CERASTES_DECREASES(2 - k)
  for (; &sm[i][j]; ) { k = k + 1; }  // <- here
  return r + k;
}

int f_dowhile_elem(int i, int j, int c)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 3)
{
  static int sm[2][3];
  int r = 0;
  int k = 0;
  CERASTES_INVARIANT(0 <= k && k <= 2)
  CERASTES_DECREASES(2 - k)
  do { k = k + 1; } while (&sm[i][j]);  // <- here
  return r + k;
}

int f_invariant_elem(int i, int j, int c)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 3)
{
  static int sm[2][3];
  int r = 0;
  int k = 0;
  CERASTES_INVARIANT(0 <= k && k <= 2 && &sm[i][j])  // <- here
  CERASTES_DECREASES(2 - k)
  while (k < 2) {
    k = k + 1;
  }
  return r + k;
}

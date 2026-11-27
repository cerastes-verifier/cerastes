// expect: rejected cerastes-array_address_null_test @ L60
// expect: rejected cerastes-array_address_null_test @ L72
// expect: rejected cerastes-array_address_null_test @ L82
// expect: rejected cerastes-array_address_null_test @ L92
// expect: rejected cerastes-array_address_null_test @ L102
// expect: rejected cerastes-array_address_null_test @ L112
// expect: rejected cerastes-array_address_null_test @ L122
// expect: rejected cerastes-array_address_null_test @ L132
// expect: rejected cerastes-array_address_null_test @ L142
// expect: rejected cerastes-array_address_null_test @ L152
// expect: rejected cerastes-array_address_null_test @ L164
// expect: rejected cerastes-array_address_null_test @ L176
// expect: rejected cerastes-array_address_null_test @ L183
// expect: rejected cerastes-array_address_null_test @ L194
// expect: rejected cerastes-array_address_null_test @ L208
// expect: rejected cerastes-array_address_null_test @ L222
// expect: rejected cerastes-array_address_null_test @ L234
// expect: rejected cerastes-array_address_null_test @ L244
// expect: rejected cerastes-array_address_null_test @ L254
// expect: rejected cerastes-array_address_null_test @ L264
// expect: rejected cerastes-array_address_null_test @ L274
// expect: rejected cerastes-array_address_null_test @ L284
// expect: rejected cerastes-array_address_null_test @ L294
// expect: rejected cerastes-array_address_null_test @ L304
// expect: rejected cerastes-array_address_null_test @ L314
// expect: rejected cerastes-array_address_null_test @ L326
// expect: rejected cerastes-array_address_null_test @ L338
// expect: rejected cerastes-array_address_null_test @ L345
// expect: rejected cerastes-unsupported_expression @ L345
// expect: rejected cerastes-array_address_null_test @ L356
// expect: rejected cerastes-unsupported_expression @ L356
// expect: rejected cerastes-array_address_null_test @ L370
// mode: c23, c++20
// twin: tests/memory/matrix_global.c
// Pins: 0064 item 5 (question 322, audit round E1: every truth test of a row or an element address is array_address_null_test, as a != 0 of a one-dimensional array is), the matrix member of a GLOBAL structure (struct U gu): gu.m[i] and &gu.m[i][j], the contract forms beside: one function per form (if, while, !, &&, ||, ?:, (bool), (_Bool), assert, CERASTES_CHECK, for, do-while, a loop invariant)
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

struct U {
  int n;
  int m[2][3];
};
struct U gu;

int f_if_row(int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns(gu) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns(gu))
{
  int r = 0;
  int k = 0;
  if (gu.m[i]) { r = 1; }  // <- here
  return r + k;
}

int f_while_row(int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns(gu) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns(gu))
{
  int r = 0;
  int k = 0;
  CERASTES_INVARIANT(0 <= k && k <= 2)
  CERASTES_DECREASES(2 - k)
  while (gu.m[i]) { k = k + 1; }  // <- here
  return r + k;
}

int f_not_row(int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns(gu) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns(gu))
{
  int r = 0;
  int k = 0;
  r = !gu.m[i];  // <- here
  return r + k;
}

int f_and_row(int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns(gu) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns(gu))
{
  int r = 0;
  int k = 0;
  r = gu.m[i] && c;  // <- here
  return r + k;
}

int f_or_row(int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns(gu) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns(gu))
{
  int r = 0;
  int k = 0;
  r = c || gu.m[i];  // <- here
  return r + k;
}

int f_cond_row(int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns(gu) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns(gu))
{
  int r = 0;
  int k = 0;
  r = gu.m[i] ? 1 : 2;  // <- here
  return r + k;
}

int f_cast_bool_row(int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns(gu) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns(gu))
{
  int r = 0;
  int k = 0;
  r = (bool)gu.m[i];  // <- here
  return r + k;
}

int f_cast_Bool_row(int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns(gu) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns(gu))
{
  int r = 0;
  int k = 0;
  r = (BOOL)gu.m[i];  // <- here
  return r + k;
}

int f_assert_row(int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns(gu) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns(gu))
{
  int r = 0;
  int k = 0;
  assert(gu.m[i]);  // <- here
  return r + k;
}

int f_check_row(int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns(gu) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns(gu))
{
  int r = 0;
  int k = 0;
  CERASTES_CHECK(gu.m[i]);  // <- here
  return r + k;
}

int f_for_row(int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns(gu) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns(gu))
{
  int r = 0;
  int k = 0;
  CERASTES_INVARIANT(0 <= k && k <= 2)
  CERASTES_DECREASES(2 - k)
  for (; gu.m[i]; ) { k = k + 1; }  // <- here
  return r + k;
}

int f_dowhile_row(int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns(gu) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns(gu))
{
  int r = 0;
  int k = 0;
  CERASTES_INVARIANT(0 <= k && k <= 2)
  CERASTES_DECREASES(2 - k)
  do { k = k + 1; } while (gu.m[i]);  // <- here
  return r + k;
}

int f_c_requires_row(int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns(gu) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns(gu))
    CERASTES_REQUIRES(gu.m[i])  // <- here
{
  (void)i;
  (void)j;
  (void)c;
  return 0;
}

int f_c_ensures_row(int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns(gu) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns(gu))
    CERASTES_ENSURES(r, r == 0 && gu.m[i])  // <- here
{
  (void)i;
  (void)j;
  (void)c;
  return 0;
}

int f_invariant_row(int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns(gu) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns(gu))
{
  int r = 0;
  int k = 0;
  CERASTES_INVARIANT(0 <= k && k <= 2 && gu.m[i])  // <- here
  CERASTES_DECREASES(2 - k)
  while (k < 2) {
    k = k + 1;
  }
  return r + k;
}

int f_if_elem(int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns(gu) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns(gu))
{
  int r = 0;
  int k = 0;
  if (&gu.m[i][j]) { r = 1; }  // <- here
  return r + k;
}

int f_while_elem(int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns(gu) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns(gu))
{
  int r = 0;
  int k = 0;
  CERASTES_INVARIANT(0 <= k && k <= 2)
  CERASTES_DECREASES(2 - k)
  while (&gu.m[i][j]) { k = k + 1; }  // <- here
  return r + k;
}

int f_not_elem(int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns(gu) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns(gu))
{
  int r = 0;
  int k = 0;
  r = !&gu.m[i][j];  // <- here
  return r + k;
}

int f_and_elem(int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns(gu) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns(gu))
{
  int r = 0;
  int k = 0;
  r = &gu.m[i][j] && c;  // <- here
  return r + k;
}

int f_or_elem(int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns(gu) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns(gu))
{
  int r = 0;
  int k = 0;
  r = c || &gu.m[i][j];  // <- here
  return r + k;
}

int f_cond_elem(int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns(gu) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns(gu))
{
  int r = 0;
  int k = 0;
  r = &gu.m[i][j] ? 1 : 2;  // <- here
  return r + k;
}

int f_cast_bool_elem(int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns(gu) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns(gu))
{
  int r = 0;
  int k = 0;
  r = (bool)&gu.m[i][j];  // <- here
  return r + k;
}

int f_cast_Bool_elem(int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns(gu) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns(gu))
{
  int r = 0;
  int k = 0;
  r = (BOOL)&gu.m[i][j];  // <- here
  return r + k;
}

int f_assert_elem(int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns(gu) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns(gu))
{
  int r = 0;
  int k = 0;
  assert(&gu.m[i][j]);  // <- here
  return r + k;
}

int f_check_elem(int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns(gu) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns(gu))
{
  int r = 0;
  int k = 0;
  CERASTES_CHECK(&gu.m[i][j]);  // <- here
  return r + k;
}

int f_for_elem(int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns(gu) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns(gu))
{
  int r = 0;
  int k = 0;
  CERASTES_INVARIANT(0 <= k && k <= 2)
  CERASTES_DECREASES(2 - k)
  for (; &gu.m[i][j]; ) { k = k + 1; }  // <- here
  return r + k;
}

int f_dowhile_elem(int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns(gu) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns(gu))
{
  int r = 0;
  int k = 0;
  CERASTES_INVARIANT(0 <= k && k <= 2)
  CERASTES_DECREASES(2 - k)
  do { k = k + 1; } while (&gu.m[i][j]);  // <- here
  return r + k;
}

int f_c_requires_elem(int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns(gu) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns(gu))
    CERASTES_REQUIRES(&gu.m[i][j])  // <- here
{
  (void)i;
  (void)j;
  (void)c;
  return 0;
}

int f_c_ensures_elem(int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns(gu) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns(gu))
    CERASTES_ENSURES(r, r == 0 && &gu.m[i][j])  // <- here
{
  (void)i;
  (void)j;
  (void)c;
  return 0;
}

int f_invariant_elem(int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns(gu) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns(gu))
{
  int r = 0;
  int k = 0;
  CERASTES_INVARIANT(0 <= k && k <= 2 && &gu.m[i][j])  // <- here
  CERASTES_DECREASES(2 - k)
  while (k < 2) {
    k = k + 1;
  }
  return r + k;
}

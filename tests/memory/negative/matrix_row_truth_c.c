// expect: rejected cerastes-array_address_null_test @ L63
// expect: rejected cerastes-array_address_null_test @ L75
// expect: rejected cerastes-array_address_null_test @ L85
// expect: rejected cerastes-array_address_null_test @ L95
// expect: rejected cerastes-array_address_null_test @ L105
// expect: rejected cerastes-array_address_null_test @ L115
// expect: rejected cerastes-array_address_null_test @ L125
// expect: rejected cerastes-array_address_null_test @ L135
// expect: rejected cerastes-array_address_null_test @ L145
// expect: rejected cerastes-array_address_null_test @ L155
// expect: rejected cerastes-array_address_null_test @ L167
// expect: rejected cerastes-array_address_null_test @ L179
// expect: rejected cerastes-array_address_null_test @ L189
// expect: rejected cerastes-array_address_null_test @ L199
// expect: rejected cerastes-array_address_null_test @ L210
// expect: rejected cerastes-array_address_null_test @ L225
// expect: rejected cerastes-array_address_null_test @ L237
// expect: rejected cerastes-array_address_null_test @ L247
// expect: rejected cerastes-array_address_null_test @ L257
// expect: rejected cerastes-array_address_null_test @ L267
// expect: rejected cerastes-array_address_null_test @ L277
// expect: rejected cerastes-array_address_null_test @ L287
// expect: rejected cerastes-array_address_null_test @ L297
// expect: rejected cerastes-array_address_null_test @ L307
// expect: rejected cerastes-array_address_null_test @ L317
// expect: rejected cerastes-array_address_null_test @ L329
// expect: rejected cerastes-array_address_null_test @ L341
// expect: rejected cerastes-array_address_null_test @ L351
// expect: rejected cerastes-array_address_null_test @ L361
// expect: rejected cerastes-unsupported_expression @ L361
// expect: rejected cerastes-array_address_null_test @ L372
// expect: rejected cerastes-unsupported_expression @ L372
// expect: frontend-warning set_but_not_used @ L152
// expect: frontend-warning set_but_not_used @ L186
// expect: frontend-warning set_but_not_used @ L314
// expect: frontend-warning set_but_not_used @ L348
// mode: c23, c++20
// twin: tests/memory/matrix_address.c
// Pins: 0064 item 5 (question 322, audit round E1: every truth test of a row or an element address is array_address_null_test, as a != 0 of a one-dimensional array is), the matrix MEMBER of a local structure (struct U { int n; int m[2][3]; } u): u.m[i] and &u.m[i][j], and through a pointer in contracts (p->m[i], &p->m[i][j]): one function per form (if, while, !, &&, ||, ?:, (bool), (_Bool), assert, CERASTES_CHECK, for, do-while, a loop invariant)
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

int f_if_row(int i, int j, int c)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 3)
{
  struct U u = {0, {{0, 0, 0}, {0, 0, 0}}};
  int r = 0;
  int k = 0;
  if (u.m[i]) { r = 1; }  // <- here
  return r + k;
}

int f_while_row(int i, int j, int c)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 3)
{
  struct U u = {0, {{0, 0, 0}, {0, 0, 0}}};
  int r = 0;
  int k = 0;
  CERASTES_INVARIANT(0 <= k && k <= 2)
  CERASTES_DECREASES(2 - k)
  while (u.m[i]) { k = k + 1; }  // <- here
  return r + k;
}

int f_not_row(int i, int j, int c)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 3)
{
  struct U u = {0, {{0, 0, 0}, {0, 0, 0}}};
  int r = 0;
  int k = 0;
  r = !u.m[i];  // <- here
  return r + k;
}

int f_and_row(int i, int j, int c)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 3)
{
  struct U u = {0, {{0, 0, 0}, {0, 0, 0}}};
  int r = 0;
  int k = 0;
  r = u.m[i] && c;  // <- here
  return r + k;
}

int f_or_row(int i, int j, int c)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 3)
{
  struct U u = {0, {{0, 0, 0}, {0, 0, 0}}};
  int r = 0;
  int k = 0;
  r = c || u.m[i];  // <- here
  return r + k;
}

int f_cond_row(int i, int j, int c)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 3)
{
  struct U u = {0, {{0, 0, 0}, {0, 0, 0}}};
  int r = 0;
  int k = 0;
  r = u.m[i] ? 1 : 2;  // <- here
  return r + k;
}

int f_cast_bool_row(int i, int j, int c)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 3)
{
  struct U u = {0, {{0, 0, 0}, {0, 0, 0}}};
  int r = 0;
  int k = 0;
  r = (bool)u.m[i];  // <- here
  return r + k;
}

int f_cast_Bool_row(int i, int j, int c)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 3)
{
  struct U u = {0, {{0, 0, 0}, {0, 0, 0}}};
  int r = 0;
  int k = 0;
  r = (BOOL)u.m[i];  // <- here
  return r + k;
}

int f_assert_row(int i, int j, int c)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 3)
{
  struct U u = {0, {{0, 0, 0}, {0, 0, 0}}};
  int r = 0;
  int k = 0;
  assert(u.m[i]);  // <- here
  return r + k;
}

int f_check_row(int i, int j, int c)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 3)
{
  struct U u = {0, {{0, 0, 0}, {0, 0, 0}}};
  int r = 0;
  int k = 0;
  CERASTES_CHECK(u.m[i]);  // <- here
  return r + k;
}

int f_for_row(int i, int j, int c)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 3)
{
  struct U u = {0, {{0, 0, 0}, {0, 0, 0}}};
  int r = 0;
  int k = 0;
  CERASTES_INVARIANT(0 <= k && k <= 2)
  CERASTES_DECREASES(2 - k)
  for (; u.m[i]; ) { k = k + 1; }  // <- here
  return r + k;
}

int f_dowhile_row(int i, int j, int c)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 3)
{
  struct U u = {0, {{0, 0, 0}, {0, 0, 0}}};
  int r = 0;
  int k = 0;
  CERASTES_INVARIANT(0 <= k && k <= 2)
  CERASTES_DECREASES(2 - k)
  do { k = k + 1; } while (u.m[i]);  // <- here
  return r + k;
}

int f_invariant_row(int i, int j, int c)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 3)
{
  struct U u = {0, {{0, 0, 0}, {0, 0, 0}}};
  int r = 0;
  int k = 0;
  CERASTES_INVARIANT(0 <= k && k <= 2 && u.m[i])  // <- here
  CERASTES_DECREASES(2 - k)
  while (k < 2) {
    k = k + 1;
  }
  return r + k;
}

int f_p_requires_row(struct U *p, int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns(*p) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_REQUIRES(p->m[i])  // <- here
{
  (void)p;
  (void)i;
  (void)j;
  (void)c;
  return 0;
}

int f_p_ensures_row(struct U *p, int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns(*p) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(r, cerastes_owns(*p) && r == 0 && p->m[i])  // <- here
{
  (void)p;
  (void)i;
  (void)j;
  (void)c;
  return 0;
}

int f_if_elem(int i, int j, int c)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 3)
{
  struct U u = {0, {{0, 0, 0}, {0, 0, 0}}};
  int r = 0;
  int k = 0;
  if (&u.m[i][j]) { r = 1; }  // <- here
  return r + k;
}

int f_while_elem(int i, int j, int c)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 3)
{
  struct U u = {0, {{0, 0, 0}, {0, 0, 0}}};
  int r = 0;
  int k = 0;
  CERASTES_INVARIANT(0 <= k && k <= 2)
  CERASTES_DECREASES(2 - k)
  while (&u.m[i][j]) { k = k + 1; }  // <- here
  return r + k;
}

int f_not_elem(int i, int j, int c)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 3)
{
  struct U u = {0, {{0, 0, 0}, {0, 0, 0}}};
  int r = 0;
  int k = 0;
  r = !&u.m[i][j];  // <- here
  return r + k;
}

int f_and_elem(int i, int j, int c)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 3)
{
  struct U u = {0, {{0, 0, 0}, {0, 0, 0}}};
  int r = 0;
  int k = 0;
  r = &u.m[i][j] && c;  // <- here
  return r + k;
}

int f_or_elem(int i, int j, int c)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 3)
{
  struct U u = {0, {{0, 0, 0}, {0, 0, 0}}};
  int r = 0;
  int k = 0;
  r = c || &u.m[i][j];  // <- here
  return r + k;
}

int f_cond_elem(int i, int j, int c)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 3)
{
  struct U u = {0, {{0, 0, 0}, {0, 0, 0}}};
  int r = 0;
  int k = 0;
  r = &u.m[i][j] ? 1 : 2;  // <- here
  return r + k;
}

int f_cast_bool_elem(int i, int j, int c)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 3)
{
  struct U u = {0, {{0, 0, 0}, {0, 0, 0}}};
  int r = 0;
  int k = 0;
  r = (bool)&u.m[i][j];  // <- here
  return r + k;
}

int f_cast_Bool_elem(int i, int j, int c)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 3)
{
  struct U u = {0, {{0, 0, 0}, {0, 0, 0}}};
  int r = 0;
  int k = 0;
  r = (BOOL)&u.m[i][j];  // <- here
  return r + k;
}

int f_assert_elem(int i, int j, int c)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 3)
{
  struct U u = {0, {{0, 0, 0}, {0, 0, 0}}};
  int r = 0;
  int k = 0;
  assert(&u.m[i][j]);  // <- here
  return r + k;
}

int f_check_elem(int i, int j, int c)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 3)
{
  struct U u = {0, {{0, 0, 0}, {0, 0, 0}}};
  int r = 0;
  int k = 0;
  CERASTES_CHECK(&u.m[i][j]);  // <- here
  return r + k;
}

int f_for_elem(int i, int j, int c)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 3)
{
  struct U u = {0, {{0, 0, 0}, {0, 0, 0}}};
  int r = 0;
  int k = 0;
  CERASTES_INVARIANT(0 <= k && k <= 2)
  CERASTES_DECREASES(2 - k)
  for (; &u.m[i][j]; ) { k = k + 1; }  // <- here
  return r + k;
}

int f_dowhile_elem(int i, int j, int c)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 3)
{
  struct U u = {0, {{0, 0, 0}, {0, 0, 0}}};
  int r = 0;
  int k = 0;
  CERASTES_INVARIANT(0 <= k && k <= 2)
  CERASTES_DECREASES(2 - k)
  do { k = k + 1; } while (&u.m[i][j]);  // <- here
  return r + k;
}

int f_invariant_elem(int i, int j, int c)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 3)
{
  struct U u = {0, {{0, 0, 0}, {0, 0, 0}}};
  int r = 0;
  int k = 0;
  CERASTES_INVARIANT(0 <= k && k <= 2 && &u.m[i][j])  // <- here
  CERASTES_DECREASES(2 - k)
  while (k < 2) {
    k = k + 1;
  }
  return r + k;
}

int f_p_requires_elem(struct U *p, int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns(*p) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_REQUIRES(&p->m[i][j])  // <- here
{
  (void)p;
  (void)i;
  (void)j;
  (void)c;
  return 0;
}

int f_p_ensures_elem(struct U *p, int i, int j, int c)
    CERASTES_REQUIRES(cerastes_owns(*p) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(r, cerastes_owns(*p) && r == 0 && &p->m[i][j])  // <- here
{
  (void)p;
  (void)i;
  (void)j;
  (void)c;
  return 0;
}

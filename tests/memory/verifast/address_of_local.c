// expect: error lifetime @ L130
// expect: error lifetime @ L137
// expect: error lifetime @ L146
// mode: c23, c++20
// Port of VeriFast's examples/address_of_local.c (~/Documents/verifast at 0fb63506) to cerastes (decision 0038, M4.5b1c1).
// Adaptation: `//@ requires integer(i, ?v)` / `ensures integer(i, v+1)` become `cerastes_owns(*i)` and `*i == cerastes_old(*i) + 1`, with the
// bound that decision 0022 asks for (nothing wraps); `//@ invariant true` becomes `CERASTES_INVARIANT(true)` with a measure; `void f()` is
// `void f(void)`; a label at the end of a block is `end: ;` (C++ needs a statement); `abort()` needs <stdlib.h>; the unused locals are used by an `assert(*ptr == 0);` (the front end warns about an unused local, and tests are warning-free), and the dead block of `test_goto` (VeriFast's `allow_dead_code`) is behind `if (c == 0)` (the front end warns about a statement it finds unreachable) and assigns where VeriFast initializes.  The three `//~ should_fail`
// lines are errors here, `lifetime` at the closing brace of the function (or of the loop body) whose local the callee kept; `destroy_half` took
// half a permission in VeriFast and takes a whole one here (decision 0037: one `owns` atom), so `dispose_half_local` and `dispose_half_local2`
// keep `destroy`'s shape.  Left out: `address_of_local` and `address_of_local_uintptr_t` (an `int **`, M4.5b1c3), `destroy_half` itself, and
// `looptrouble` (an unsound VeriFast example, a heap of `void *`).  The `native-ok` check is not asked.
// Line map (this file : VeriFast's):
//   21 : 1   28 : 8   35 : 15   44 : 64   61 : 77   73 : 89   85 : 102   97 : 115   109 : 129
//   123 : 145   130 : 158 (should_fail)   132 : 167   137 : 174 (should_fail)   139 : 176   146 : 185 (should_fail)   149 : 189
#include <cerastes.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

void inc(int *i)
    CERASTES_REQUIRES(cerastes_owns(*i) && *i < 2147483647)
    CERASTES_ENSURES(cerastes_owns(*i) && *i == cerastes_old(*i) + 1)
{
  (*i) = (*i) + 1;
}

void inc_uintptr_t(uintptr_t *i)
    CERASTES_REQUIRES(cerastes_owns(*i) && *i < 1000)
    CERASTES_ENSURES(cerastes_owns(*i) && *i == cerastes_old(*i) + 1)
{
  (*i) = (*i) + 1;
}

void address_of_param(int x)
{
  x = 5;
  int *ptr = &x;
  inc(ptr);
  int z = x;
  assert(z == 6);
}

void test_goto(void)
{
  int c = 0;
  if (c == 0) {
    goto end;
  }
  {
    int x;
    int *p;
    p = &x;
    *p = 5;
    abort();
  }
end:
  ;
}

void test_goto2(void)
{
  {
    int x = 0;
    int *ptr = &x;
    assert(*ptr == 0);
    goto end;
  }
end:
  ;
}

void test_goto3(void)
{
  {
    int x = 0;
    int *ptr = &x;
    assert(*ptr == 0);
    goto next;
  next:
    x = 3;
  }
}

void test_break(void)
{
  CERASTES_INVARIANT(true)
  CERASTES_DECREASES(0)
  while (true) {
    int x = 0;
    int *ptr = &x;
    assert(*ptr == 0);
    break;
  }
}

void test_break2(void)
{
  CERASTES_INVARIANT(true)
  CERASTES_DECREASES(0)
  while (true) {
    int x = 0;
    int *ptr = &x;
    assert(*ptr == 0);
    break;
  }
}

void test_requires_ensures_loop(void)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 5)
  CERASTES_DECREASES(5 - i)
  while (i < 5) {
    int x = 0;
    int *ptr = &x;
    assert(*ptr == 0);
    i = i + 1;
  }
  assert(i == 5);
}

void destroy(int *i)
    CERASTES_REQUIRES(cerastes_owns(*i));

void dispose_local(void)
{
  int x = 0;
  destroy(&x);
}

void dispose_half_local(int y)
{
  int x = 0;
  destroy(&x);
  destroy(&y);
}

void dispose_half_local2(void)
{
  CERASTES_INVARIANT(true)
  CERASTES_DECREASES(0)
  while (true) {
    int x = 0;
    destroy(&x);
  }
}

void break_statement(void)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 1)
  CERASTES_DECREASES(1 - i)
  while (i < 1) {
    int x = 0;
    int *ptr = &x;
    assert(*ptr == 0);
    break;
  }
}

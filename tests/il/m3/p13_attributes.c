// expect: golden
// mode: c23
// M3 probe P13 (C23): recognized attributes in verified code, the C spellings.
#include <stdnoreturn.h>
[[noreturn]] void nr_attr(int);
_Noreturn void nr_spec(int);
noreturn void nr_macro(int);
__attribute__((noreturn)) void nr_gnu(int);
[[unsequenced]] int unseq(int a);
[[reproducible]] int repro(int a);
[[nodiscard]] int nd(int a);
[[deprecated]] int dep(int a);
[[maybe_unused]] static int mu(int a) { return a; }
int const_fn(int a) __attribute__((const));
int pure_fn(int a) __attribute__((pure));
int stmts(int x) {
  [[maybe_unused]] int u = 0;
  [[assume(x > 0)]];
  switch (x) { case 1: x = 2; [[fallthrough]]; default: break; }
  [[]];
  return x;
}

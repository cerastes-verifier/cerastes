// expect: rejected cerastes-unsupported_declaration @ L11
// mode: c++20
// twin: tests/memory/array_sum_two.c
// Pins: 0059 item 2 (P78 update): in C++ a bound that names a local constant (const int N = 4; int a[N];) is rejected: the bound is read from its written expression and N is a name (lifted by M4.5e1e; in C the same declaration is a variable length array, reject_vla.c)
// Pre-registered from the P78 update of record 0059 item 2
#include <cerastes.h>

int f(void)
{
  const int N = 4;
  [[maybe_unused]] int a[N];
  return N;
}

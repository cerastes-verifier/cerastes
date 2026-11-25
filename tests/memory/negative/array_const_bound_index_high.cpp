// expect: error out-of-bounds @ L13
// mode: c++20
// twin: tests/memory/array_const_bound.cpp
// Pins: 0063 item 14: the array of int a[N] with const int N = 4 has the bound 4, N's own checked constant: i == 4 reaches the read (a bound read as larger would verify)
// Pre-registered from record 0063 item 14 (Part D), before any run: the one error
#include <cerastes.h>

int f(int i)
    CERASTES_REQUIRES(0 <= i && i <= 4)
{
  const int N = 4;
  int a[N] = {1, 2, 3, 4};
  return a[i];  // <- here
}

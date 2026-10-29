// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0038 items 2 and 4: a cell and a pointer in a for loop with a step, a do loop and a loop with a continue path (each form's back edge)
#include <cerastes.h>

int via_for(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
    CERASTES_ENSURES(r, r == n)
{
  int x = 0;
  int *p = &x;
  int i;
  CERASTES_INVARIANT(0 <= i && i <= n && x == i)
  CERASTES_DECREASES(n - i)
  for (i = 0; i < n; i += 1) {
    *p += 1;
  }
  return x;
}

int via_do(int n)
    CERASTES_REQUIRES(1 <= n && n <= 100)
    CERASTES_ENSURES(r, r == n)
{
  int x = 0;
  int *p = &x;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i < n && x == i)
  CERASTES_DECREASES(n - i)
  do {
    *p += 1;
    i += 1;
  } while (i < n);
  return x;
}

int via_continue(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
    CERASTES_ENSURES(r, r <= n)
{
  int x = 0;
  int *p = &x;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && x <= i)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    i += 1;
    if (i == 7) {
      continue;
    }
    *p += 1;
  }
  return x;
}

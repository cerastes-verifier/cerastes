// expect: golden
// expect: native-ok
// Pins: decision 0013 item 1: decreases repeated on a redeclaration (it must match), and omitted on one
#include <cerastes.h>


int f(int n) CERASTES_REQUIRES(n >= 0) CERASTES_DECREASES(n);
int f(int n) CERASTES_REQUIRES(n >= 0) CERASTES_DECREASES(n);
int f(int n);

int f(int n)
{
  return n == 0 ? 0 : f(n - 1);
}

int g(int k) CERASTES_DECREASES(k + 1);
int g(int m) CERASTES_DECREASES(m + 1)
{
  return m;
}

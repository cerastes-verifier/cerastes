// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0027 items 1 and 4: loops that flip and set bits of a variable by a constant mask in every iteration, with invariants about the value after i iterations
#include <cerastes.h>

int flip_k_times(int n, int k)
    CERASTES_REQUIRES(0 <= n && 0 <= k && k <= 10)
    CERASTES_ENSURES(r, (k % 2 == 0 && r == n) || (k % 2 == 1 && r == (n - n % 256) + (255 - n % 256)))
{
  int x = n;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= k && ((i % 2 == 0 && x == n) || (i % 2 == 1 && x == (n - n % 256) + (255 - n % 256))))
  CERASTES_DECREASES(k - i)
  while (i < k) {
    x = x ^ 255;
    i = i + 1;
  }
  return x;
}

int set_nibble_k_times(int n, int k)
    CERASTES_REQUIRES(0 <= n && 0 <= k && k <= 10)
    CERASTES_ENSURES(r, r >= n && (k == 0 || r % 16 == 15))
{
  int x = n;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= k && x >= n && (i == 0 || x % 16 == 15))
  CERASTES_DECREASES(k - i)
  while (i < k) {
    x |= 15;
    i = i + 1;
  }
  return x;
}

// expect: error invariant-preserved @ L28
// mode: c23, c++20
// twin: tests/scalar/mask_or_xor_loop.c
// Pins: 0027 items 1 and 4: x |= 7 sets 3 bits, so x % 16 == 15 is not preserved (n == 0, i == 0: x becomes 7)
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
    x |= 7;
    i = i + 1;
  }
  return x;
}

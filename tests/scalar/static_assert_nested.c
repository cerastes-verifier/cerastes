// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0025 item 5 (question 40): block-scope static assertions in a loop body, a switch body, nested blocks and a branch are all checked (the cross-check finds every one in each encoded body)
#include <cerastes.h>

// an assertion in a loop body
int count(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
    CERASTES_ENSURES(r, r == n)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    static_assert(sizeof(int) == 4, "int is 32 bits");
    i++;
  }
  return i;
}

// an assertion in a nested block of a switch body
int pick(int k)
    CERASTES_REQUIRES(0 <= k && k < 3)
    CERASTES_ENSURES(r, r == k + 10)
{
  switch (k) {
  case 0: {
    static_assert((1 << 30) == 1073741824, "a shift that fits");
    return 10;
  }
  case 1:
    {
      {
        static_assert((signed char)127 == 127, "a cast that fits");
      }
      return 11;
    }
  default:
    break;
  }
  return 12;
}

// an assertion in a block of a for loop and one in a branch
int total(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
    CERASTES_ENSURES(r, r == n)
{
  int s = 0;
  int i;
  CERASTES_INVARIANT(0 <= i && i <= n && s == i)
  CERASTES_DECREASES(n - i)
  for (i = 0; i < n; i++) {
    if (n > 0) {
      static_assert(sizeof(char) == 1, "char is one byte");
    }
    s++;
  }
  return s;
}

// expect: verified
// expect: native-ok
// mode: c23, c++20
// Covers: 0056 item 11 (question 214, focused review r5): a `return` inside an `if` in an inner loop, a struct local with an owning leaf (NULL) that neither loop names: the exit out of the loops keeps the struct from being framed, and the function returns with nothing owned
// Pre-registered from the review's r5_nested_return.c (verified on F2; not yet run in this task): verifies in both modes [Corrected after the first run (question 160): `s` was set but never used (EDG #550 and GCC -Werror=unused-but-set-variable), so the final `return 1;` reads `s.data` (`return s.data == NULL ? 1 : 0;`; reading `s.n` instead gave an overflow, because a struct local's int member loses its value across such a loop, also on the base); the verdict is otherwise unchanged.]
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

int f(int n)
    CERASTES_REQUIRES(0 <= n && n <= 3)
{
  struct buf s;
  s.n = 0;
  s.data = NULL;
  int k = 0;
  int *pk = &k;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    int j = 0;
    CERASTES_INVARIANT(0 <= j && j <= 2)
    CERASTES_DECREASES(2 - j)
    while (j < 2) {
      *pk = j;
      if (i == n) {
        if (j == 1) {
          return 0;
        }
      }
      j = j + 1;
    }
    i = i + 1;
  }
  return s.data == NULL ? 1 : 0;
}

// expect: verified
// expect: native-ok
// mode: c23, c++20
// Covers: 0056 item 11 (question 214, audit re-audit r3): a struct local with an owning leaf, NULL, that the loop does not name, and a loop with a `return` in it: the struct is not framed out of a loop with an exit, so the early return does not strand a lifetime row; the function returns with nothing owned
// Pre-registered from the re-audit's r3_return_in_loop_noname.c and edg-backend report 5 (verified, both modes, 2-3 s): verifies in both modes [Corrected after the first run (question 160): `s` was set but never used (EDG #550 and GCC -Werror=unused-but-set-variable), so the final `return 1;` reads `s.data` (`return s.data == NULL ? 1 : 0;`; reading `s.n` instead gave an overflow, because a struct local's int member loses its value across such a loop, also on the base); the verdict is otherwise unchanged.]
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
    *pk = i;
    if (i == n) {
      return 0;
    }
    i = i + 1;
  }
  return s.data == NULL ? 1 : 0;
}

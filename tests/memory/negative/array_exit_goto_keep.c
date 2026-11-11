// expect: error lifetime @ L19
// expect: error lifetime @ L21
// mode: c23, c++20
// twin: tests/memory/array_exit_goto.c
// Pins: 0059 item 6: a callee kept a[1]; each way out of the block reports lifetime: the goto, and the closing brace of the path that falls through
// Pre-registered from the M4.5e1a sweep (report 5, s11 keep): two errors, the jump and the brace
#include <cerastes.h>

void dispose(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p));

int leave(int c)
    CERASTES_ENSURES(r, r == 0)
{
  {
    int a[2] = {1, 2};
    dispose(&a[1]);
    if (c != 0) {
      goto out;
    }
  }
out:
  return 0;
}

// expect: golden
// Contrast probe: the walker reports a routine only if its position is in the
// primary file.  After `#line 100 "elsewhere.h"` the function `moved` has a
// position in a different file name, so it is left out.  That loss must be
// visible, not silent: the walker prints a warning line for the
// [[cerastes::...]] attribute it dropped, and the `U` count in the final
// `end walk:` line (routines with bodies in non-system files outside the
// primary file) counts `moved`.  `before` is still in the primary file and is
// reported.  The golden records exactly what is visible; if either signal
// disappears the probe fails.

int before(int x) { return x + 1; }

#line 100 "elsewhere.h"
int moved(int x) {
  [[cerastes::lost(x > 0)]];
  return x * 2;
}

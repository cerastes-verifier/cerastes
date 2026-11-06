// expect: error permission @ L17
// mode: c23, c++20
// twin: tests/memory/static_local_result.c
// Pins: 0052 item 2 (audit S1, c_cov_live_null.c): live(r) is covered by the static local's implicit owns(s), but one path returns NULL: the pure rows (r != null) are guarded by the atom's guard and not by the cover, so the NULL return fails them: permission at the return (it was exit 3)
// Pre-registered from audit round 2 (build/m4.5d2b/audit2.md) S1: permission at the return NULL
#include <cerastes.h>
#include <stddef.h>

int *where(int c)
    CERASTES_ENSURES(r, cerastes_live(r))
{
  static int s;
  s = 1;
  if (c) {
    return &s;
  }
  return NULL;  // <- here
}

int main(void)
{
  int *p = where(0);
  return p == NULL;
}

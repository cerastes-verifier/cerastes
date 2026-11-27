// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 7 and 0060 item 13: owning(n) on a parameter that points to structures adds p == NULL || owns_range(p, n) to both lists: the callee writes p[0].x under the null test, callers pass a local array of structures and NULL; and unguarded, the program of the retired pin range_reject_struct_owning_count.c AS IT STANDS (its f_c06: owning(n) beside the same unguarded owns_range(p, n) in a requires, body (void)p; here also (void)n, which the native -Wextra compile needs because the annotation macros expand to nothing). The scalar analog verifies (the orchestrator's probe build/m4.5e1e2b/q341/scalar_owning_same.c: f_same), so both rows of that pin (L14, L15) retire with this program (Flips A, C1). The merged form (p == NULL || owns_range(p, n)) is the scalar positive's merged
// Pre-registered from record 0064 items 7, 8 and 10 (Part C1), before any run: verifies
#include <cerastes.h>
#include <stddef.h>

struct P {
  int x;
  int y;
};

void zero_first(int n, struct P *p CERASTES_OWNING_N(n))
{
  if (p != NULL && n > 0) {
    p[0].x = 0;
  }
}

int use(void)
    CERASTES_ENSURES(r, r == 0)
{
  struct P v[3] = {{1, 2}, {3, 4}, {5, 6}};
  zero_first(3, v);
  zero_first(0, NULL);
  return 0;
}

void merged(int n, struct P *p CERASTES_OWNING_N(n))
    CERASTES_REQUIRES(p == NULL || cerastes_owns_range(p, n))
    CERASTES_ENSURES(p == NULL || cerastes_owns_range(p, n))
{
  (void)n;
  (void)p;
}

void unguarded(int n, struct P *p CERASTES_OWNING_N(n))
    CERASTES_REQUIRES(cerastes_owns_range(p, n))
{
  (void)n;
  (void)p;
}

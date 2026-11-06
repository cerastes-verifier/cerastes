// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0052 item 3 Loops, shape L6: a non-owning alias a = v of an owning loop variable: the loop's place for a is the owner's own block, so writes through a (also after a = v in the body, beside a write through v, and with a call in the body) verify, and v is freed after the loop
// Pre-registered from record 0052 item 3 Loops (il-analyst's probes build/m4.5d2b/loop/p): verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

void touch(void) {}

int through_alias(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
{
  CERASTES_OWNING int *v = (int *)malloc(sizeof(int));
  if (v == NULL) {
    return 1;
  }
  *v = 0;
  int *a = v;
  int k = n;
  CERASTES_INVARIANT(0 <= k && k <= n)
  CERASTES_DECREASES(k)
  while (k > 0) {
    k -= 1;
    *a = k;
  }
  free(v);
  return 0;
}

int alias_assigned_in_body(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
{
  CERASTES_OWNING int *v = (int *)malloc(sizeof(int));
  if (v == NULL) {
    return 1;
  }
  *v = 0;
  int *a = v;
  int k = n;
  CERASTES_INVARIANT(0 <= k && k <= n)
  CERASTES_DECREASES(k)
  while (k > 0) {
    k -= 1;
    a = v;
    *a = k;
  }
  free(v);
  return 0;
}

int alias_and_owner_written(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
{
  CERASTES_OWNING int *v = (int *)malloc(sizeof(int));
  if (v == NULL) {
    return 1;
  }
  *v = 0;
  int *a = v;
  int k = n;
  CERASTES_INVARIANT(0 <= k && k <= n)
  CERASTES_DECREASES(k)
  while (k > 0) {
    k -= 1;
    *a = k;
    *v = k;
  }
  free(v);
  return 0;
}

int alias_with_call(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
{
  CERASTES_OWNING int *v = (int *)malloc(sizeof(int));
  if (v == NULL) {
    return 1;
  }
  *v = 0;
  int *a = v;
  int k = n;
  CERASTES_INVARIANT(0 <= k && k <= n)
  CERASTES_DECREASES(k)
  while (k > 0) {
    k -= 1;
    *a = k;
    touch();
  }
  free(v);
  return 0;
}

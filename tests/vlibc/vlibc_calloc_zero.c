// expect: verified
// expect: native-ok
// mode: c23
// c-only: vlibc_malloc creates no C++ objects (record 0050 item 7)
// Pins: 0050 items 2, 3 (0047 item 2): vlibc_calloc has calloc's contract: an integer and every leaf of a structure are zero
// Pre-registered from record 0050 item 2: verifies
#include <vlibc/stdlib.h>
#include <cerastes.h>

struct pair {
  int a;
  int b;
};

int zeroed_int(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  int *p = (int *)vlibc_calloc(1, sizeof(int));
  if (p == NULL) {
    return -1;
  }
  int v = *p;
  vlibc_free(p);
  return v;
}

int zeroed_pair(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  struct pair *s = (struct pair *)vlibc_calloc(1, sizeof(struct pair));
  if (s == NULL) {
    return -1;
  }
  CERASTES_CHECK(s->a == 0);
  CERASTES_CHECK(s->b == 0);
  int v = s->a + s->b;
  vlibc_free(s);
  return v;
}

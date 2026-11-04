// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0047 item 2 (null being all bits zero is a platform assumption, true of arm64 macOS and x86-64 Linux): calloc zeroes every leaf of the structure: an integer 0, false, a null pointer, the enumeration's 0
// Pre-registered from record 0047 item 2 (one value clause per leaf): verifies
#include <cerastes.h>
#include <stdlib.h>

enum color { RED, GREEN };

struct cfg {
  int n;
  bool on;
  int *p;
  enum color c;
};

int zeroed_struct(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  struct cfg *s = (struct cfg *)calloc(1, sizeof(struct cfg));
  if (s == NULL) {
    return -1;
  }
  CERASTES_CHECK(s->n == 0);
  CERASTES_CHECK(!s->on);
  CERASTES_CHECK(s->p == NULL);
  CERASTES_CHECK(s->c == RED);
  free(s);
  return 0;
}

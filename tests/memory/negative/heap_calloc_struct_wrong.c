// expect: error check @ L25
// mode: c23, c++20
// twin: tests/memory/heap_calloc_struct.c
// Pins: 0047 item 2: the enumeration leaf is 0, which is RED, not GREEN
// Pre-registered from record 0047 item 2: check at the CERASTES_CHECK
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
  CERASTES_CHECK(s->c == GREEN);  // <- here
  free(s);
  return 0;
}

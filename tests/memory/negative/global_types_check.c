// expect: error check @ L15
// mode: c23, c++20
// twin: tests/memory/global_types.c
// Pins: 0041 item 2: col starts as GREEN (5), not RED
#include <cerastes.h>

enum color { RED, GREEN = 5 };

static enum color col = GREEN;
static unsigned char uc = 200;

int main(void)
{
  uc += 55;
  CERASTES_CHECK(col == RED);
  return uc + (col == GREEN ? 1 : 0);
}

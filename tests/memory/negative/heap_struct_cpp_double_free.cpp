// expect: error invalid-free @ L22
// twin: tests/memory/heap_struct.cpp
// Pins: 0047 item 6 (C++): a double free of a static_cast allocation
// Pre-registered from record 0047 item 6: invalid-free at the second free; re-registered after audit round 1 (record 0047 item 6): one invalid-free, no lifetime
#include <cerastes.h>
#include <stdlib.h>

struct S {
  int x;
  int y;
};

int use_struct()
{
  S *s = static_cast<S *>(malloc(sizeof(S)));
  if (s == nullptr) {
    return -1;
  }
  s->x = 1;
  s->y = 2;
  free(s);
  free(s);  // <- here
  return 0;
}

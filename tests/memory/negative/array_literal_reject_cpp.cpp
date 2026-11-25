// expect: rejected cerastes-unsupported_expression @ L9
// twin: tests/memory/array_decltype.cpp
// Pins: 0063 item 12 (P82 (e)): C++20 accepts array compound literals with no diagnostic at all (a GNU extension), so the rejection is cerastes's, as for the struct literal's address (the tag is 0044's; to confirm at the joined run)
// Pre-registered from record 0063 item 12 (Part D), before any run: rejected
#include <cerastes.h>

int f()
{
  int *p = (int[]){1, 2, 3};  // <- here
  return p[0];
}

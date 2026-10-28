// expect: error postcondition @ L41
// mode: c23, c++20
// twin: tests/memory/null_spellings.c
// Pins: 0032 item 7: !d is false for &x, so only seven of the eight tests count
#include <stddef.h>
#include <cerastes.h>

int spellings(void)
    CERASTES_ENSURES(r, r == 8)
{
  int x = 1;
  int *a = NULL;
  int *b = 0;
  int *c = nullptr;
  int *d = &x;
  int n = 0;
  if (!a) {
    n += 1;
  }
  if (!b) {
    n += 1;
  }
  if (!c) {
    n += 1;
  }
  if (a == NULL) {
    n += 1;
  }
  if (b == 0) {
    n += 1;
  }
  if (c == nullptr) {
    n += 1;
  }
  if (d != NULL) {
    n += 1;
  }
  if (!d) {
    n += 1;
  }
  return n;
}

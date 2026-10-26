// expect: rejected cerastes-empty_body @ L10
// mode: c23, c++20
// twin: tests/scalar/warn_accepted_neighbours.c
// Pins: 0025 item 6 R13: `switch (x);`
#include <cerastes.h>

int test(int x, int n)
    CERASTES_REQUIRES(0 <= x && x < 100 && 0 <= n && n < 100)
{
  switch (x);
  return x;
}

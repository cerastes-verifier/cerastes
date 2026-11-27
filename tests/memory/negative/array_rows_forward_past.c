// expect: error permission @ L21
// expect: error permission @ L21
// mode: c23, c++20
// twin: tests/memory/array_rows_forward.c
// Pins: 0064 item 13 (question 361): forwarding with one row more than the function holds (n + 1) fails the callee's shape row at the call (permission) and, with it, the row of every cell of every row (a second permission error at the same line, one cause: record item 7 gives a forwarder that passes more than it holds two permission errors at the call, the shape row and the first leaf's marked row, where a local array one short draws one; the two-expect form was settled with question 368 and is a correction under question 277: a twin with several real errors lists every one)
// Pre-registered from record 0064 item 13 (Part E), before any run: permission at the call
#include <cerastes.h>

void g(int a[][3], int n)
    CERASTES_REQUIRES(cerastes_owns_range(a, n) && n > 0 && n < 100)
    CERASTES_ENSURES(cerastes_owns_range(a, n) && a[0][0] == 1)
{
  (void)n;
  a[0][0] = 1;
}

void f(int a[][3], int n)
    CERASTES_REQUIRES(cerastes_owns_range(a, n) && n > 0 && n < 100)
    CERASTES_ENSURES(cerastes_owns_range(a, n) && a[0][0] == 1)
{
  g(a, n + 1);  // <- here
}

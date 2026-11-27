// expect: error permission @ L15
// mode: c23, c++20
// twin: tests/memory/matrix_decl_called.c
// Pins: 0064 item 13 (audit round E1, finding 4; the auditor's e_bodiless_called_short): the same declaration called with three rows of a two-row matrix: the call needs to read m[0..3) and the caller does not hold it (`permission` at the call)
// Pre-registered from record 0064 item 13 (Part E), before any run: permission at the call
#include <cerastes.h>

int first(int a[][3], int n)
    CERASTES_REQUIRES(cerastes_reads_range(a, n) && n > 0)
    CERASTES_ENSURES(r, cerastes_reads_range(a, n) && r == a[0][0]);

int use(void)
{
  int m[2][3] = {{1, 2, 3}, {4, 5, 6}};
  return first(m, 3);  // <- here
}

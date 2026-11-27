// expect: error postcondition @ L16
// expect: error out-of-bounds @ L23
// mode: c23, c++20
// twin: tests/memory/array_rows_spellings.c
// Pins: 0064 item 13 (question 320 and P84 (b); the coordinator's ruling on question 358 (ii)): in its own function each, int (*a)[3] returns a cell no postcondition fixes (the value 2 is not known: postcondition at the return), and row *a (the typedef) indexes its second index to 3: out-of-bounds, which shows that the typedef's bound 3 is the one read for the row's length
// Pre-registered from record 0064 item 13 (Part E), before any run: postcondition at the first return, out-of-bounds at the second function's read (one row per path)
#include <cerastes.h>

typedef int row[3];

int star(int (*a)[3], int n)
    CERASTES_REQUIRES(cerastes_reads_range(a, n) && n > 0)
    CERASTES_ENSURES(r, cerastes_reads_range(a, n) && r == 2)
{
  (void)n;
  return a[0][1];  // <- here
}

int typedefed(row *a, int n, int j)
    CERASTES_REQUIRES(cerastes_reads_range(a, n) && n > 0 && 0 <= j && j <= 3)
{
  (void)n;
  return a[0][j];  // <- here
}

// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 13 (audit round E1, finding 4; a called declaration WITHOUT a body that has a row parameter, the auditor's e_bodiless_called): the caller relies on the declaration's ensures (the result equals a[0][0], so the caller knows it is 1) and on the reads_range given back; the cells of a declaration that only owns the range are the caller's to read (the ones the contract states: none), and the controls are negative/matrix_decl_called_short.c and negative/matrix_decl_called_unstated.c
// Pre-registered from record 0064 item 13 (Part E), before any run: verifies
#include <cerastes.h>

int first(int a[][3], int n)
    CERASTES_REQUIRES(cerastes_reads_range(a, n) && n > 0)
    CERASTES_ENSURES(r, cerastes_reads_range(a, n) && r == a[0][0]);

int use(void)
    CERASTES_ENSURES(r, r == 1)
{
  int m[2][3] = {{1, 2, 3}, {4, 5, 6}};
  return first(m, 2);
}

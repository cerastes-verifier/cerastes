// (L16, a local matrix declared with an initializer, was rejected here until Part E lifted it: tests/memory/matrix_local.c; the line stays)
// (L17, a matrix used in code, m[1][2], was rejected here until Part E lifted it: tests/memory/matrix_local.c; the line stays)
// (L20, a global matrix, was rejected here until Part E lifted it: tests/memory/matrix_global.c; the line stays)
// (L26, an `int a[2][3]` parameter, drew a declaration and a type row here until Part E lifted them: tests/memory/array_rows_spellings.c, its w1; the line stays)
// (L40, p[i].a[j] through a pointer to ONE structure with an array member, was rejected here until Part E lifted it: tests/memory/array_rows_struct_elem_index_one.c; the line stays)
// expect: rejected cerastes-unsupported_expression @ L28
// mode: c23, c++20
// twin: tests/memory/array_sum_two.c
// Pins: Flips A (E) (plan section 9), applied after the joined proof j4 (build/m4.5e1e2d/rej-j4.txt): the staging pin of the base build (f07d80a) had rows for a matrix used in code (m[1][2]), a global matrix, an int a[2][3] parameter and p[i].a[j] through a pointer to one structure; Part E lifted them and their programs are the positives named in the comments above. What stays is the row Part E's encoder prints at L28: `(void)a;` of the `int a[2][3]` parameter, a bare use of a row parameter (record 0064 item 13; the same rule as negative/array_rows_reject_bare.c's `discard(int a[][3])`, for this spelling). No depth-3 row was ever in this file; the depth-3 rows are negative/matrix_reject_slice4_staging.c's
// Pre-registered from the base build's R2 run, before any part of M4.5e1e2: rejected (observed, not predicted)
#include <cerastes.h>
#include <stddef.h>
#include <stdlib.h>
int f_e02(void)
{
    int m[2][3] = {{1, 2, 3}, {4, 5, 6}}; // <- here
    return m[1][2]; // <- here
}

int gm[2][3]; // <- here
int f_e09(void)
{
    return 0;
}

int f_e14(int a[2][3]) // <- here
{
    (void)a;
    return 0;
}

struct T {
    int a[4];
    int n;
};
int f_e25(struct T *p, int i, int j)
    CERASTES_REQUIRES(cerastes_owns(*p) && i == 0 && j == 0)
    CERASTES_ENSURES(r, cerastes_owns(*p))
{
    return p[i].a[j]; // <- here
}

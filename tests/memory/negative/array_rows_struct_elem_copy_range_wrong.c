// expect: error permission @ L26
// expect: error postcondition @ L35
// expect: error out-of-bounds @ L41
// expect: error permission @ L41
// expect: error out-of-bounds @ L48
// expect: error permission @ L48
// expect: error postcondition @ L57
// expect: error permission @ L63
// expect: error postcondition @ L71
// mode: c23, c++20
// twin: tests/memory/array_rows_struct_elem_copy_range.c
// Pins: 0064 item 12 (audit E3a finding 8 (a); the auditor's c05 rd, keep, past, pastsrc, symw and d08b, d08c, measured both modes): a copy writes its destination, so a destination under reads_range is `permission`; the copy takes the source element's cells, so a claim that the destination keeps its own (keeps_wrong, symbolic_wrong) or equals another element (from_wrong) fails at the closing brace; p[n] on either side is outside the owned range (`out-of-bounds`, and the cells of the element past the range are not owned: `permission`, two rows on the line)
// Pre-registered (test-mutator-10, Task 1/2, before any run): rows as the auditor measured them: write_reads permission at the copy; keeps_wrong, symbolic_wrong, from_wrong postcondition at the closing brace (void functions); past_dest and past_source out-of-bounds and permission on one line each; into_reads permission; right-value siblings: cp (old(p[0].a[1])), cpsym (old(p[j].a[2])), from_reads (s[1].a[2]) in tests/memory/array_rows_struct_elem_copy_range.c
#include <cerastes.h>

struct W {
  int a[3];
  int n;
};

void write_reads(struct W *p, int n)
    CERASTES_REQUIRES(cerastes_reads_range(p, n) && n >= 2)
    CERASTES_ENSURES(cerastes_reads_range(p, n))
{
  (void)n;
  p[1] = p[0];  // <- here
}

void keeps_wrong(struct W *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && n >= 2)
    CERASTES_ENSURES(cerastes_owns_range(p, n) && p[1].a[1] == cerastes_old(p[1].a[1]))
{
  (void)n;
  p[1] = p[0];
}  // <- here

void past_dest(struct W *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && n >= 2)
    CERASTES_ENSURES(cerastes_owns_range(p, n))
{
  p[n] = p[0];  // <- here
}

void past_source(struct W *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && n >= 2)
    CERASTES_ENSURES(cerastes_owns_range(p, n))
{
  p[0] = p[n];  // <- here
}

void symbolic_wrong(struct W *p, int n, int i, int j)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && 0 <= i && i < n && 0 <= j && j < n)
    CERASTES_ENSURES(cerastes_owns_range(p, n) && p[i].a[2] == cerastes_old(p[i].a[2]))
{
  (void)n;
  p[i] = p[j];
}  // <- here

void into_reads(struct W *d, struct W *s)
    CERASTES_REQUIRES(cerastes_owns(*d) && cerastes_reads_range(s, 2))
    CERASTES_ENSURES(cerastes_owns(*d) && cerastes_reads_range(s, 2))
{
  s[1] = *d;  // <- here
}

void from_wrong(struct W *d, struct W *s)
    CERASTES_REQUIRES(cerastes_owns(*d) && cerastes_reads_range(s, 2))
    CERASTES_ENSURES(cerastes_owns(*d) && cerastes_reads_range(s, 2) && d->a[2] == s[0].a[2])
{
  *d = s[1];
}  // <- here

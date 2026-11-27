// expect: error postcondition @ L34
// expect: error postcondition @ L48
// expect: error postcondition @ L61
// expect: error out-of-bounds @ L68
// expect: error uninitialized @ L81
// expect: error invariant-preserved @ L93
// mode: c23, c++20
// twin: tests/memory/array_rows_struct_elem_loop.c
// Pins: 0064 items 3, 9 and 10 (audit E3a finding 8 (f); the auditor's l01, l02, l03, l05, l05d, l05e, measured, both modes): a loop that writes a cell of a local element through a pointer, writes v[i].a[0] for every i, or calls a function that may change a cell loses that cell (the postcondition about it fails: unknown by design, a loop that writes nothing keeps it); an invariant that reads v[1].a[i + 1] may read past the member (`out-of-bounds`), reads a cell never given a value (`uninitialized`), or claims a cell the body writes (`invariant-preserved`)
// Pre-registered (test-mutator-10, Task 1/2, before any run): three `postcondition` rows at the returns, then out-of-bounds, uninitialized and invariant-preserved at the invariant lines; controls: reach, reads_call and invariant_reads in tests/memory/array_rows_struct_elem_loop.c (the same programs minus the write, the call to a writer and the cause of each invariant row); corrected once after the first probe in lane edg (question 160, program only, no expect line moves): the last function reported a third row, `overflow` at its return, because v[0].n is not known after a loop whose invariant is not preserved, so the return no longer subtracts 1
#include <cerastes.h>

struct P {
  int n;
  int a[3];
};

int wr1i(int *q)
    CERASTES_REQUIRES(cerastes_owns(*q))
    CERASTES_ENSURES(r, cerastes_owns(*q) && r == 0);

int loop_writes_through_a_pointer(void)
    CERASTES_ENSURES(r, r == 8)
{
  struct P v[2] = {{1, {2, 3, 4}}, {5, {6, 7, 8}}};
  int *q = &v[1].a[2];
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    *q = i;
    i = i + 1;
  }
  return v[1].a[2];  // <- here
}

int loop_writes_elements(void)
    CERASTES_ENSURES(r, r == 2)
{
  struct P v[2] = {{1, {2, 3, 4}}, {5, {6, 7, 8}}};
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 2)
  CERASTES_DECREASES(2 - i)
  while (i < 2) {
    v[i].a[0] = 0;
    i = i + 1;
  }
  return v[0].a[0];  // <- here
}

int loop_calls_a_writer(void)
    CERASTES_ENSURES(r, r == 8)
{
  struct P v[2] = {{1, {2, 3, 4}}, {5, {6, 7, 8}}};
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    i = i + 1 + wr1i(&v[1].a[2]);
  }
  return v[1].a[2];  // <- here
}

int invariant_past_the_member(void)
{
  struct P v[2] = {{1, {2, 3, 4}}, {5, {6, 7, 8}}};
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 2 && v[1].a[i + 1] >= 0)  // <- here
  CERASTES_DECREASES(2 - i)
  while (i < 2) {
    i = i + 1;
  }
  return v[0].n - 1;
}

int invariant_reads_unset(void)
{
  struct P v[2];
  v[0].n = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 2 && v[1].a[1] >= 0)  // <- here
  CERASTES_DECREASES(2 - i)
  while (i < 2) {
    i = i + 1;
  }
  return v[0].n;
}

int invariant_not_kept(void)
{
  struct P v[2] = {{1, {2, 3, 4}}, {5, {6, 7, 8}}};
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 2 && v[1].a[1] == 7)  // <- here
  CERASTES_DECREASES(2 - i)
  while (i < 2) {
    v[1].a[1] = 0;
    i = i + 1;
  }
  return v[0].n;
}

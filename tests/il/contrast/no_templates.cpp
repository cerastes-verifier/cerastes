// expect: golden
// Contrast probe: rules out a vacuous G1 pass in the other direction.  A file
// with only non-template functions (loops, arithmetic, casts) must show
// `plain` routines and nothing that belongs to templates: no `prototype`,
// `instance` or `specialization` routines, no `template` or `constraint`
// lines, no `op` lines (those appear only in prototype bodies) and no
// `tconst` lines.  If the walker reported such lines here, the template
// checks of G1 would pass whatever the IL contained.

int add(int a, int b) { return a + b; }

long widen(short s) { return (long)s * 2 + (long)1; }

unsigned narrow(long v) {
  unsigned total = 0;
  for (long i = 0; i < v; ++i) {
    total += static_cast<unsigned>(i % 7);
  }
  return total;
}

int count_down(int n) {
  int steps = 0;
  while (n > 0) {
    n = n - 1;
    ++steps;
  }
  return steps;
}

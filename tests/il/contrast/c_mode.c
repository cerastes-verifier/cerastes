// expect: golden
// mode: c23
// Contrast probe: in C mode the walker must report only `plain` routines: a
// struct, functions that take and return it, and a loop.  There must be no
// `prototype`, `instance`, `specialization`, `template`, `constraint`, `op`
// or `tconst` line.  No `.ti` file may appear in the working directory: this
// was checked by hand by running the walker from the repository root and
// from an empty directory and listing the directory afterwards (the
// configuration has no template instantiation output, and none was created).

struct point {
  int x;
  int y;
};

struct point make_point(int x, int y) {
  struct point p = {x, y};
  return p;
}

int manhattan(struct point p) {
  int dx = p.x < 0 ? -p.x : p.x;
  int dy = p.y < 0 ? -p.y : p.y;
  return dx + dy;
}

int sum_to(int n) {
  int total = 0;
  for (int i = 0; i < n; ++i) {
    total += i;
  }
  return total;
}

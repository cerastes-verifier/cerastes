// expect: golden
// mode: c23
// Walker smoke test, C mode: a plain function, a static function, a
// declaration-only function and a function taking a struct pointer.
struct point {
  int x;
  int y;
};

static int sq(int v) { return v * v; }

int dot(const struct point* a, const struct point* b) {
  return a->x * b->x + a->y * b->y;
}

int extern_decl(int n, char c);

int use(void) {
  struct point p = {1, 2};
  return sq(3) + dot(&p, &p);
}

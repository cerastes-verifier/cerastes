// expect: golden
// Contrast probe: a function template that is never used must still have a
// `prototype` routine with a body (G1 check 2) and must have no `instance`
// line, because nothing instantiated it.  `plain_user` is an ordinary
// function that does not call the template, so an `instance` line anywhere
// in the golden would mean the walker invents instances.  The prototype of
// `unused` must show its `op` lines, which proves the body was kept.

template <typename T>
T unused(T v) {
  return v + 1;
}

int plain_user(int x) { return x * 2; }

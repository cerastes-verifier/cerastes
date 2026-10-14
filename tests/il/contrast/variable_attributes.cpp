// expect: golden
// Contrast probe: attributes on entities other than routines and statements.
// Each [[unreg_ns::...]] below is on exactly one entity, named by its
// attribute: v1 on a local variable, c1 on a class, f1 on a field, n1 on a
// namespace.  The walker's output has one `attr` line per attribute or none;
// never two.  Which of them the walker reports is recorded in the golden, not
// assumed (the walker reports routines and statements; a variable, class,
// field or namespace may be outside what it looks at).  `use` is there so the
// local variable has a function to live in.

namespace [[unreg_ns::n1(1)]] ns {
  int inside(int x) { return x; }
}

struct [[unreg_ns::c1(2)]] Point {
  [[unreg_ns::f1(3)]] int x;
  int y;
};

int use(int a) {
  [[unreg_ns::v1(a > 0)]] int local = a + 1;
  return local + ns::inside(a);
}

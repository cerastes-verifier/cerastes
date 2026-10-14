// expect: golden
// Contrast probe: statements and operations in nested bodies must be reported
// under their own routine and nowhere else.  Each body below has an
// attributed null statement, named by what it sits in, and an operation:
//   in_plain / in_plain's lambda          a_plain, a_plain_lambda
//   in_tmpl / in_tmpl's lambda            a_tmpl, a_tmpl_lambda  (prototype
//                                         bodies: `op` lines too)
//   Local::member, a local class in       a_local_member
//   in_plain
// A `stmt` or `op` line that shows up under the wrong routine (the lambda's
// under the enclosing function, or the enclosing function's under the lambda)
// is the failure this probe exists to catch.  What the walker reports for the
// lambda closures and the local class is recorded in the golden, not assumed.

int in_plain(int x) {
  [[unreg_ns::a_plain(x)]];
  int y = x * 3;
  auto lam = [](int v) {
    [[unreg_ns::a_plain_lambda(v)]];
    return v - 7;
  };
  struct Local {
    int member(int w) {
      [[unreg_ns::a_local_member(w)]];
      return w << 1;
    }
  };
  return lam(y) + Local().member(y);
}

template <typename T>
T in_tmpl(T x) {
  [[unreg_ns::a_tmpl(x)]];
  T y = x * 3;
  auto lam = [](T v) {
    [[unreg_ns::a_tmpl_lambda(v)]];
    return v - 7;
  };
  return lam(y) + x;
}

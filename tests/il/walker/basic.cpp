// expect: golden
// Walker smoke test: plain routines in namespaces, a class with member
// functions (one implicit constructor), overloads, a static function, a
// declaration-only function and a lambda.
namespace outer {
namespace inner {
int twice(int x) { return x * 2; }
long twice(long x) { return x * 2; }
}  // namespace inner

static int helper(int a, int b) { return a - b; }
}  // namespace outer

int declared_only(int x, const char* s);

struct Counter {
  int n;
  int next() { return ++n; }
  int get() const { return n; }
  static int zero() { return 0; }
};

struct Pair {
  int a;
  int b;
  Pair() : a(0), b(0) {}
  Pair(int x, int y) : a(x), b(y) {}
};

struct Holder {
  Pair p;
};

int use(int v) {
  Counter c{};
  Pair p(1, 2);
  Holder h;
  auto add = [v](int w) { return v + w; };
  return outer::inner::twice(v) + outer::helper(v, 1) + c.next() + p.a + h.p.b + add(1);
}

// expect: golden
// Gate G1 check 5: unrecognized [[cerastes::...]] attributes survive in the
// IL with namespace, name, raw tokens and position.
// PASS means each name below is reported with its namespace "cerastes", its
// own name, its raw argument tokens and its position, attached to the entity
// or statement named here. Warning #2803 (unrecognized namespace) is expected
// for every cerastes:: attribute before M2.
//   a1  prefix of the non-template declaration of f
//   a2  after the parameter list of g (declaration and definition)
//   a3  on the while loop in g;  a4 on the for loop in g;  a5 on the null
//       statement in g
//   b1  prefix of the function template declaration of tmpl (the plain
//       function h also carries a prefix attribute, b0)
//   b2  after the parameter list of tmpl (declaration and definition)
//   b3, b4, b5  while, for and null statement in the body of tmpl: on the
//       prototype routine and, where the instance has a body, on the instance
//       tmpl<int> that use() creates
//   c1  after the parameter list of the member function Box<T>::get
//   c2, c3  while loop and null statement in the body of Box<T>::get: on the
//       prototype member and on Box<int>::get
// Whether each placement survives is the question; the golden records the
// answer, it does not assume it.

[[cerastes::a1(x > 0, y + 1)]] int f(int x);

int g(int x) [[cerastes::a2(x > 0, x + 1)]];

int g(int x) [[cerastes::a2(x > 0, x + 1)]] {
  int i = 0;
  [[cerastes::a3(i < x, i + 2)]] while (i < x) { ++i; }
  [[cerastes::a4(i <= x)]] for (int j = 0; j < x; ++j) { ++i; }
  [[cerastes::a5(i > 0)]];
  return i;
}

[[cerastes::b0(x > 0, y + 1)]] int h(int x);

template <typename T>
[[cerastes::b1(x > 0, x + 1)]] T tmpl(T x);

template <typename T>
T tmpl(T x) [[cerastes::b2(x > 0, x + 1)]];

template <typename T>
T tmpl(T x) [[cerastes::b2(x > 0, x + 1)]] {
  T i = 0;
  [[cerastes::b3(i < x, i + 2)]] while (i < x) { ++i; }
  [[cerastes::b4(i <= x)]] for (T j = 0; j < x; ++j) { ++i; }
  [[cerastes::b5(i > 0)]];
  return i;
}

template <typename T>
struct Box {
  T v;
  T get(T x) [[cerastes::c1(x > 0, x + 1)]] {
    T i = 0;
    [[cerastes::c2(i < x)]] while (i < x) { ++i; }
    [[cerastes::c3(i > 0)]];
    return i + v;
  }
};

int use() {
  Box<int> b{1};
  return tmpl(3) + b.get(2);
}

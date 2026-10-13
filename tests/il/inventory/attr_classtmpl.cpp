// expect: golden
// Inventory probe for check 5: prefix attributes inside a class template.
// PASS: each of m1..m3 is reported with its tokens on the entity named below,
// in the prototype (Box<T>) and in the instance (Box<int>): m1 as
// `on routine`, m2 as top-level `attr ... on field Box<T>::v`, m3 as top-level
// `attr ... on type Box<T>` / `on type Box<int>`; or the IL shows it is
// dropped (the golden records which).
//   m1  prefix of the member function declaration get1
//   m2  prefix of the data member v
//   m3  after the class-key of the class template itself
template <typename T>
struct [[cerastes::m3(1)]] Box {
  [[cerastes::m2(2)]] T v;
  [[cerastes::m1(x > 0)]] T get1(T x) { return x; }
};
int use() { Box<int> b{1}; return b.get1(2); }

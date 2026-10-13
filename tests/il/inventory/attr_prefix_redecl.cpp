// expect: golden
// Inventory probe for check 5a: prefix attributes on redeclarations.
// PASS means every attribute below is reported on the routine of its entity
// (prototype and instance for the template); the golden records the result.
//   r1, r2  plain function declared twice, one prefix attribute on each
//   r3, r4  function template declared twice, one on each, then defined and
//           instantiated by use()
//   r5, r6  member function declared in the class with r5, defined outside
//           the class with r6
[[cerastes::r1(1)]] int p1(int x);
[[cerastes::r2(2)]] int p1(int x);
int p1(int x) { return x; }
template <typename T> [[cerastes::r3(3)]] T p2(T x);
template <typename T> [[cerastes::r4(4)]] T p2(T x);
template <typename T> T p2(T x) { return x; }
struct S { [[cerastes::r5(5)]] int m(int x); };
[[cerastes::r6(6)]] int S::m(int x) { return x; }
int use() { S s; return p1(1) + p2(2) + s.m(3); }

// expect: golden
// Inventory probe for check 5 / gotchas: where an attribute after the
// parameter list lands when its declaration is the only one of the entity.
// PASS: p1..p4 are each reported with their tokens on the entity named here,
// or the IL shows they are dropped (the golden records which).
//   p1  declaration only, non-template
//   p2  definition only, non-template
//   p3  function template, definition only (instantiated by use())
//   p4  function template, declaration only
//   p5, p6  function template declared with p5, then redeclared and defined
//       with p6 (the redeclaration case of check5's tmpl)
int s1(int x) [[unreg_ns::p1(x > 0)]];
int s2(int x) [[unreg_ns::p2(x > 0)]] { return x; }
template <typename T> T s3(T x) [[unreg_ns::p3(x > 0)]] { return x; }
template <typename T> T s4(T x) [[unreg_ns::p4(x > 0)]];
template <typename T> T s5(T x) [[unreg_ns::p5(x > 0)]];
template <typename T> T s5(T x) [[unreg_ns::p6(x > 1)]] { return x; }
int use() { return s2(1) + s3(2) + s5(3); }

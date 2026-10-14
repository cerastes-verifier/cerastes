// M5 spike: where can an unrecognized attribute be placed around a concept?
// Compile with --c++20. Which placements EDG accepts and records is shown by
// --cerastes_walk (attr lines) and by the diagnostics.
template <class T, class U> concept same_as = __is_same(T, U);

template <class T>
concept Addable = requires(T a, T b) {
  { a + b } -> same_as<T>;
};

// (1) before the template header, (2) after the word concept, and attributes
//     inside a requires-expression are REJECTED: see concept_placements_rejected.cpp
// (3) after the concept name: accepted and recorded ("on template C3")
template <class T> concept C3 [[cerastes::p2]] = true;
// (3) on a function template constrained by a requires-clause
template <class T> requires Addable<T>
[[cerastes::p3]] T add(T a, T b) { return a + b; }
// (4) after the parameter list of a constrained function template
template <class T> requires Addable<T>
T add2(T a, T b) [[cerastes::p4]] { return a + b; }

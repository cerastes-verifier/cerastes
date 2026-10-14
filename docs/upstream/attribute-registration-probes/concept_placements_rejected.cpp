// Each placement below is rejected by EDG (run with --c++20 --no_code_gen and
// read the diagnostics). The grammar has no attribute-specifier-seq in these
// positions. (An attribute after the concept name is allowed: concept_placements.cpp.)
template <class T> concept same_as_int = __is_same(T, int);
[[cerastes::q1]] template <class T> concept C1 = true;                  // (1) before the template header
template <class T> concept [[cerastes::q2]] C2 = true;                  // (2) after the word concept
// (3) after the concept name is ACCEPTED: see concept_placements.cpp
template <class T> concept C4 = requires(T a) { [[cerastes::q4]] a + a; }; // (4) a simple requirement
template <class T> concept C5 = requires(T a) { { a + a } [[cerastes::q5]]; }; // (5) a compound requirement

// expect: golden
// Annotation-free copy of tests/cxx/templates/integral_per_instantiation.cpp from
// verifast branch generic2, commit 0fb63506a51cae559ae92348eb54ba7f7be0bf6d.
// Whole-line //@ annotations are replaced by empty lines and trailing
// //~ should_fail markers are removed; no other change.
// Line offset: line N here is line N-6 in the original (header is 6 lines).
// The generic proof of a function template constrained by std::integral does
// not cover bool. Templates that use their integral type parameters in ways
// the generic proof does not support are verified per instantiation.

#include <concepts>

// Verified once for every integer type other than bool. The specialization
// for bool is verified separately.
template <std::integral T>
T same(const T v)


{
  return v;
}

// An operand of type int that is not a literal.
template <std::integral T>
T add(const T v, int n)


{
  return (T)(v + n);
}

// Bitwise operators depend on the width and signedness of T.
template <std::integral T>
T low_bit(const T v)


{
  return (T)(v & 1);
}

// An implicit conversion back to T.
template <std::integral T>
T inc(const T v)


{
  return v + 1;
}

int main()


{
  int i = same(1);

  bool b = same(true);

  long l = add(3L, 4);

  int z = low_bit(3);
  int w = inc(41);

  return 0;
}

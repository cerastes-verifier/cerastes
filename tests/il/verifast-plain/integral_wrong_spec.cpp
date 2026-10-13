// expect: golden
// Annotation-free copy of tests/cxx/templates/integral_wrong_spec.cpp from
// verifast branch generic2, commit 0fb63506a51cae559ae92348eb54ba7f7be0bf6d.
// Whole-line //@ annotations are replaced by empty lines and trailing
// //~ should_fail markers are removed; no other change.
// Line offset: line N here is line N-6 in the original (header is 6 lines).
// A function template constrained by std::integral is verified once, so its
// proof must hold for every integer type other than bool: the smallest and
// largest ones, signed and unsigned ones.

#include <concepts>

// Wrong for signed char: inc(150) does not fit.
template <std::integral T>
T inc(const T v)


{
  return (T)(v + 1);
}

// Wrong for unsigned types: dec(0) does not fit.
template <std::integral T>
T dec(const T v)


{
  return (T)(v - 1);
}

// Wrong for long: v * v can overflow.
template <std::integral T>
T square(const T v)


{
  return (T)(v * v);
}

// Wrong for unsigned types.
template <std::integral T>
void negative_one(const T v)


{

}

// Wrong for signed char.
template <std::integral T>
T two_hundred(const T v)


{
  return (T)200;
}

// The facts about the limits of T are consistent.
template <std::integral T>
void consistent(const T v)


{

}

// What EDG already does with an attribute whose arguments are expressions
// (Clang's enable_if, sig "(X,sn)"): this, lambdas, dependent expressions.
struct C {
  int val;
  int get() const __attribute__((enable_if(this->val > 0, "val must be positive")));
};
int lam(int n) __attribute__((enable_if([](int i) { return i > 0; }(n), "n > 0 via a lambda")));
template <class T>
T tf(T v) __attribute__((enable_if(v + 1 > 0, "dependent")));
template <class T>
T tf(T v) __attribute__((enable_if(v + 1 > 0, "dependent"))) { return v; }
int use() { return tf(2); }

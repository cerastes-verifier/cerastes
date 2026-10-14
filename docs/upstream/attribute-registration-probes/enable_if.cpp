int g(int x) __attribute__((enable_if(x > 0, "x must be positive")));
int g(int x) __attribute__((enable_if(x > 0, "x must be positive"))) { return x; }
template <class T>
T f(T v) __attribute__((enable_if(v > 0, "v must be positive")));
template <class T>
T f(T v) __attribute__((enable_if(v > 0, "v must be positive"))) { return v; }
int use() { return g(1) + f(2); }

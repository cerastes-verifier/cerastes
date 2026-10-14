// Reduced reproduction: attributes written after a function's parameter list.
// Compile as C++20.
int g(int x) [[cerastes::req(x > 0)]];     // line 3: declaration
int g(int x) [[cerastes::req(x < 100)]];   // line 4: redeclaration
int g(int x) { return x; }

template <class T>
T f(T v) [[cerastes::req(v > 0)]];         // line 8: template declaration
template <class T>
T f(T v) [[cerastes::req(v < 100)]] { return v; }  // line 10: definition

int use() { return g(1) + f(2); }

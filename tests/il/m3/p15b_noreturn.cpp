// expect: frontend-warning noreturn_function_does_return @ L7
// expect: frontend-warning noreturn_function_does_return @ L8
// expect: frontend-warning noreturn_function_does_return @ L9
// expect: frontend-warning noreturn_function_does_return @ L10
// mode: c++20
// M3 probe P15b: what EDG says about a return, or reaching the end, in a [[noreturn]] function.
[[noreturn]] void nr_return(int a) { if (a) return; for (;;) {} }
[[noreturn]] void nr_falls(int a) { (void)a; }
[[noreturn]] int nr_int(int a) { return a; }
[[noreturn]] void nr_loop(int a) { while (a) { a--; } }
void caller(int a) { nr_falls(a); a = 1; }

// expect: rejected cerastes-unsupported_declaration @ L11
// expect: rejected cerastes-unsupported_declaration @ L12
// expect: rejected cerastes-unsupported_declaration @ L21
// twin: tests/scalar/if_else.c
// Pins: 0012 item 12: a coroutine is rejected (its promise and return types are classes)
// Three diagnostics: the two classes and the function's return type; co_return itself is never reached.
#include <cerastes.h>

#include <coroutine>

struct task {
  struct promise_type {
    task get_return_object() { return {}; }
    std::suspend_never initial_suspend() noexcept { return {}; }
    std::suspend_never final_suspend() noexcept { return {}; }
    void return_void() {}
    void unhandled_exception() {}
  };
};

task run() { co_return; }

// (included by selftest.cpp) Loops and jumps over memory (decision 0038, M4.5b1c1): the automatic loop invariant (every place held, the
// values a loop keeps and the ones a write havocs, the flag), the iteration checks at the back edge, memory in invariants, measures and
// conditions with their framing, scope exits at `break`, `continue`, `goto` and `return`, and the rejections that stay.
// PRE-REGISTERED from the record (item numbers below): each expectation names the category and the VIR node whose position the record gives
// (the jump, the body's closing brace, the invariant or measure clause, the condition's read, the return), so the cases test the position map
// as well.  Each rule has a case that verifies and a twin that fails for one reason (invariant 2).  The seven `mem_rej_*` cases of b1a and
// b1b for loops and jumps (item 2 and 7 lift them) are here as lowering cases.
namespace {

void define_loops_and_jumps() {
  auto PT = [](Kit& k) { return k.ptr(k.i32); };
  auto N = [](Kit& k, const char* v) { return k.num(k.i32, v); };
  using LF = LoopForm;
  // `p`'s formal with its spelling for messages.
  auto FP = [](Kit& k, Function* f, int i) { return k.b.text(k.F(f, i), f->params[static_cast<size_t>(i)].name); };
  auto owns = [=](Kit& k, Function* f, int i, int amount) { return k.b.atom(AtomKind::Owns, FP(k, f, i), amount, k.at()); };
  auto reads = [=](Kit& k, Function* f, int i, int amount) { return k.b.atom(AtomKind::Reads, FP(k, f, i), amount, k.at()); };
  auto star = [=](Kit& k, Function* f, int i) { return k.deref(FP(k, f, i)); };  // `*p` in a contract
  // A variable `x` of a block, read as a variable and as `*p` of a pointer variable.
  auto rd = [=](Kit& k, const Var* p) { return k.deref(k.pn(p)); };
  // void dispose(int *p) requires owns(*p);   (trusted: the callee keeps the cell)
  auto make_dispose = [=](Kit& k) -> Function* {
    Function* d = k.fn("dispose", k.vd, {{"p", PT(k)}});
    k.req(d, owns(k, d, 0, 0));
    k.trusted(d);
    return d;
  };
  // void inc(int *p) requires owns(*p) && *p < 100 ensures owns(*p) && *p == old(*p) + 1;   (trusted: a callee that owns and gives back)
  auto make_inc = [=](Kit& k) -> Function* {
    Function* d = k.fn("inc", k.vd, {{"p", PT(k)}});
    k.req(d, k.land(owns(k, d, 0, 0), k.lt(star(k, d, 0), N(k, "100"))));
    k.ens(d, k.land(owns(k, d, 0, 0), k.eq(star(k, d, 0), k.add(k.old(star(k, d, 0)), N(k, "1")))));
    k.trusted(d);
    return d;
  };
  // void poke(int *p) requires owns(*p) ensures owns(*p);   (the cell comes back with an unknown value)
  auto make_poke = [=](Kit& k) -> Function* {
    Function* d = k.fn("poke", k.vd, {{"p", PT(k)}});
    k.req(d, owns(k, d, 0, 0));
    k.ens(d, owns(k, d, 0, 0));
    k.trusted(d);
    return d;
  };
  // int peek(const int *p) requires reads(*p) ensures reads(*p) && r == *p;   (the cell comes back with its value)
  auto make_peek = [=](Kit& k) -> Function* {
    Function* d = k.fn("peek", k.i32, {{"p", PT(k)}});
    k.req(d, reads(k, d, 0, 0));
    k.ens(d, k.land(reads(k, d, 0, 0), k.eq(k.R(d), star(k, d, 0))));
    k.trusted(d);
    return d;
  };
  // A function `f(n)` of `int` result whose body declares the cell `x` (= `init`), a pointer `p` to it and `i` (= n), then `rest`.
  const auto cell_fn = [=](Kit& k, const char* init, const std::function<std::vector<const Stmt*>(Kit&, Function*, Var*, Var*, Var*)>& rest,
                           const char* post) {
    Function* f = k.fn("f", k.i32, {{"n", k.i32}});
    k.req(f, k.range(k.F(f, 0), k.i32, "0", "100"));
    if (post != nullptr) k.ens(f, k.eq_c(k.R(f), k.i32, post));
    Var* x = k.cell(f, "x", k.i32);
    Var* p = k.local(f, "p", PT(k));
    Var* i = k.local(f, "i", k.i32);
    std::vector<const Stmt*> body = {k.decl(x, N(k, init)), k.decl(p, k.addr(x)), k.decl(i, k.pv(f, 0))};
    for (const Stmt* s : rest(k, f, x, p, i)) body.push_back(s);
    f->body = k.blkc(body);
    k.done(f);
    return f;
  };
  // `while (i > 0) body` with the invariant 0 <= i <= 100 and the measure i (the clauses of the shapes below), and the decrement.
  auto count_down = [=](Kit& k, Var* i, std::vector<Clause> extra, const Stmt* body) {
    std::vector<Clause> inv = {k.cl(k.range(k.v(i), k.i32, "0", "100"))};
    for (const Clause& c : extra) inv.push_back(c);
    return k.whl(k.gt(k.v(i), N(k, "0")), inv, {k.cl(k.v(i))}, body);
  };
  auto step = [=](Kit& k, Var* i) { return k.asg(i, k.sub(k.v(i), N(k, "1"))); };

  // ---- the seven shapes that b1a and b1b rejected (0032 item 10), now accepted (items 2, 3, 7) ----
  // while (i > 0) { if (p != NULL) i -= 1; } return r;
  const auto pointer_node = [=](Kit& k, bool wrong) -> SrcLoc {
    SrcLoc at;
    cell_fn(k, "0", [&](Kit& k, Function*, Var*, Var* p, Var* i) {
      const Stmt* r = k.ret(wrong ? N(k, "1") : N(k, "0"));
      at = r->loc;
      return std::vector<const Stmt*>{count_down(k, i, {}, k.blk({k.iff(k.ne(k.pn(p), k.nul(PT(k))), k.blk({step(k, i)}))})), r};
    }, "0");
    return at;
  };
  lc("mem_loop_with_a_pointer_node", "while (i > 0) { if (p != NULL) i -= 1; }: a loop holds a pointer node, the loop is given the cell", [=](Kit& k) { pointer_node(k, false); });
  lc("mem_loop_with_a_pointer_node_wrong", "the same returning 1 where the contract says 0", [=](Kit& k) { k.fail("postcondition", pointer_node(k, true)); });
  // while (i > 0) { x = 1; i -= 1; } return x;   (x is written by name: its value is only its range, the invariant says 0 or 1)
  const auto cell_write = [=](Kit& k, bool wrong) -> SrcLoc {
    SrcLoc at;
    Function* f = k.fn("f", k.i32, {{"n", k.i32}});
    k.req(f, k.range(k.F(f, 0), k.i32, "0", "100"));
    k.ens(f, wrong ? k.eq_c(k.R(f), k.i32, "1") : k.range(k.R(f), k.i32, "0", "1"));
    Var* x = k.cell(f, "x", k.i32);
    Var* p = k.local(f, "p", PT(k));
    Var* i = k.local(f, "i", k.i32);
    const Stmt* r = k.ret(k.v(x));
    at = r->loc;
    f->body = k.blkc({k.decl(x, N(k, "0")), k.decl(p, k.addr(x)), k.decl(i, k.pv(f, 0)),
                      count_down(k, i, {k.cl(k.range(k.v(x), k.i32, "0", "1"))}, k.blk({k.asg(x, N(k, "1")), step(k, i)})), r});
    k.done(f);
    (void)p;
    return at;
  };
  lc("mem_loop_with_a_cell", "while (i > 0) { x = 1; i -= 1; } return x: a cell written by name in a loop, its value in the invariant", [=](Kit& k) { cell_write(k, false); });
  lc("mem_loop_with_a_cell_wrong", "the same promising r == 1: with n == 0 the loop does not run and x is 0", [=](Kit& k) { k.fail("postcondition", cell_write(k, true)); });
  // while (i > 0) { i -= *p; } return 0;   (x == 1 or 0: a read-only cell keeps its value)
  const auto deref_step = [=](Kit& k, bool wrong) -> SrcLoc {
    SrcLoc at;
    cell_fn(k, wrong ? "0" : "1", [&](Kit& k, Function*, Var*, Var* p, Var* i) {
      std::vector<Clause> inv = {k.cl(k.range(k.v(i), k.i32, "0", "100"))};
      Clause measure = k.cl(k.v(i));
      at = measure.loc;
      const Stmt* w = k.whl(k.gt(k.v(i), N(k, "0")), inv, {measure}, k.blk({k.asg(i, k.sub(k.v(i), rd(k, p)))}));
      return std::vector<const Stmt*>{w, k.ret(N(k, "0"))};
    }, "0");
    return at;
  };
  lc("mem_loop_with_a_dereference", "while (i > 0) { i -= *p; } with x == 1: the read-only cell keeps its value, the measure decreases", [=](Kit& k) { deref_step(k, false); });
  lc("mem_loop_with_a_dereference_wrong", "the same with x == 0: the measure does not decrease", [=](Kit& k) { k.fail("termination", deref_step(k, true)); });
  // int *q = NULL; while (i > 0) { q = &x; i -= 1; } return (wrong ? *q : 0);
  const auto address_in_loop = [=](Kit& k, bool wrong) -> SrcLoc {
    SrcLoc at;
    cell_fn(k, "0", [&](Kit& k, Function* f, Var* x, Var*, Var* i) {
      Var* q = k.local(f, "q", PT(k));
      const Expr* d = k.deref(k.pn(q));
      at = d->loc;
      return std::vector<const Stmt*>{k.decl(q, k.nul(PT(k))), count_down(k, i, {}, k.blk({k.asg(q, k.addr(x)), step(k, i)})),
                                      k.ret(wrong ? d : N(k, "0"))};
    }, nullptr);
    return at;
  };
  lc("mem_loop_with_an_address", "q = &x inside a loop: the address of a cell is taken in the body", [=](Kit& k) { address_in_loop(k, false); });
  lc("mem_loop_with_an_address_wrong", "*q after the loop where q may still be NULL (n == 0): null-dereference, and (the check assumed) no permission", [=](Kit& k) {
    const SrcLoc at = address_in_loop(k, true);
    k.fail("null-dereference", at);
    k.fail("permission", at);
  });
  // while (i > 0) { if (i == 50) { [dispose(p);] return 1; } i -= 1; } return 0;
  const auto return_in_loop = [=](Kit& k, bool consumed) -> SrcLoc {
    SrcLoc at;
    Function* dispose = make_dispose(k);
    cell_fn(k, "0", [&](Kit& k, Function*, Var*, Var* p, Var* i) {
      const Stmt* r = k.ret(N(k, "1"));
      at = r->loc;
      std::vector<const Stmt*> then;
      if (consumed) then.push_back(k.es(k.call(dispose, {k.pn(p)})));
      then.push_back(r);
      return std::vector<const Stmt*>{count_down(k, i, {}, k.blk({k.iff(k.eq(k.v(i), N(k, "50")), k.blk(then)), step(k, i)})), k.ret(N(k, "0"))};
    }, nullptr);
    return at;
  };
  lc("mem_return_in_a_loop_with_a_live_cell", "a return inside a loop while the cell of an enclosing block is live: the loop holds the cell, the return exits it", [=](Kit& k) { return_in_loop(k, false); });
  lc("mem_return_in_a_loop_with_a_consumed_cell", "the callee kept the cell, so the return's scope exit fails at the return", [=](Kit& k) { k.fail("lifetime", return_in_loop(k, true)); });
  // { int x = a; int *p = &x; if (*p > 5) { [dispose(p);] goto done; } total = x; } done: return total;
  const auto goto_out = [=](Kit& k, bool consumed) -> SrcLoc {
    Function* dispose = make_dispose(k);
    Function* f = k.fn("f", k.i32, {{"a", k.i32}});
    k.req(f, k.range(k.F(f, 0), k.i32, "-100", "100"));
    Var* total = k.local(f, "total", k.i32);
    Var* x = k.cell(f, "x", k.i32);
    Var* p = k.local(f, "p", PT(k));
    const int id = k.jid();
    const Stmt* jump = k.go(id);
    std::vector<const Stmt*> then;
    if (consumed) then.push_back(k.es(k.call(dispose, {k.pn(p)})));
    then.push_back(jump);
    const Stmt* inner = k.blkc({k.decl(x, k.pv(f, 0)), k.decl(p, k.addr(x)), k.iff(k.gt(rd(k, p), N(k, "5")), k.blk(then)), k.asg(total, k.v(x))});
    f->body = k.blk({k.decl(total, N(k, "0")), inner, k.lab(id), k.ret(k.v(total))});
    k.done(f);
    return jump->loc;
  };
  lc("mem_goto_out_of_a_cell_block", "an outward goto leaves a block that holds a cell: the scope exit is at the goto", [=](Kit& k) { goto_out(k, false); });
  lc("mem_goto_out_of_a_consumed_cell_block", "the callee kept the cell, so the goto's scope exit fails at the goto", [=](Kit& k) { k.fail("lifetime", goto_out(k, true)); });
  // switch (code) { int x; int *p; case 0: p = &x; *p = 5; total = x; break; case 1: { int y = 3; int *q = &y; total = *q; [dispose(q);] break; } default: total = 1; }
  const auto switch_break = [=](Kit& k, bool consumed) -> SrcLoc {
    Function* dispose = make_dispose(k);
    Function* f = k.fn("f", k.i32, {{"code", k.i32}});
    Var* total = k.local(f, "total", k.i32);
    Var* x = k.cell(f, "x", k.i32);
    Var* p = k.local(f, "p", PT(k));
    Var* y = k.cell(f, "y", k.i32);
    Var* q = k.local(f, "q", PT(k));
    const int id = k.jid();
    const Stmt* inner_break = k.brk(id);
    std::vector<const Stmt*> in_case = {k.decl(y, N(k, "3")), k.decl(q, k.addr(y)), k.asg(total, rd(k, q))};
    if (consumed) in_case.push_back(k.es(k.call(dispose, {k.pn(q)})));
    in_case.push_back(inner_break);
    const Stmt* body = k.blkc({k.decl(x, nullptr), k.decl(p, nullptr), k.cs(N(k, "0")), k.asg(p, k.addr(x)), k.store(k.pn(p), N(k, "5")),
                               k.asg(total, k.v(x)), k.brk(id), k.cs(N(k, "1")), k.blkc(in_case), k.dflt(), k.asg(total, N(k, "1"))});
    f->body = k.blk({k.decl(total, N(k, "0")), k.sw(k.pv(f, 0), body, id), k.ret(k.v(total))});
    k.done(f);
    return inner_break->loc;
  };
  lc("mem_break_out_of_a_switch_with_a_cell", "a break out of a switch whose body declares a cell and out of a block inside a case: it exits only the blocks nested in the body", [=](Kit& k) { switch_break(k, false); });
  lc("mem_break_out_of_a_switch_with_a_consumed_cell", "the callee kept the case's cell, so the break out of its block fails at the break", [=](Kit& k) { k.fail("lifetime", switch_break(k, true)); });

  // ---- item 4: the iteration check at the back edge ----
  // while (i > 0) { <body>; i -= 1; }   the body is `inc(&x)` (a callee that owns and gives back: the permission is found again), or `dispose(&x)`
  const auto back_edge = [=](Kit& k, int mode) -> SrcLoc {  // 0: inc, 1: dispose
    SrcLoc at;
    Function* callee = mode == 0 ? make_inc(k) : make_dispose(k);
    cell_fn(k, "0", [&](Kit& k, Function* f, Var* x, Var*, Var* i) {
      std::vector<Clause> extra;
      if (mode == 0) extra.push_back(k.cl(k.eq(k.add(k.v(x), k.v(i)), k.pv(f, 0))));  // x + i == n
      const Stmt* body = k.blkc({k.es(k.call(callee, {k.addr(x)})), step(k, i)});
      at = body->end_loc;
      return std::vector<const Stmt*>{count_down(k, i, extra, body), k.ret(N(k, "0"))};
    }, nullptr);
    return at;
  };
  lc("mem_loop_back_edge_found", "inc(&x) in a loop: the callee owns the cell and gives it back, the iteration ends with the permission it began with", [=](Kit& k) { back_edge(k, 0); });
  lc("mem_loop_back_edge_consumed", "dispose(&x) in a loop: the iteration ends without the permission to x it began with, reported at the body's closing brace", [=](Kit& k) { k.fail("permission", back_edge(k, 1)); });

  // ---- item 3: the places of the parameters ----
  // void fill(int *p, int n) requires owns(*p) && 0 <= n <= 100 [reads(*p) && *p == 0]  ensures owns(*p) && *p == n { i = 0; [*p = 0;] while (i < n) inv i <= n && *p == i { *p = *p + 1; i += 1; } }
  const auto fill = [=](Kit& k, bool reads_only) -> SrcLoc {
    SrcLoc at;
    Function* f = k.fn("fill", k.vd, {{"p", PT(k)}, {"n", k.i32}});
    const auto atom = [&]() { return reads_only ? reads(k, f, 0, 0) : owns(k, f, 0, 0); };
    k.req(f, k.land(atom(), k.range(k.F(f, 1), k.i32, "0", "100")));
    if (reads_only) k.req(f, k.eq_c(star(k, f, 0), k.i32, "0"));
    k.ens(f, reads_only ? atom() : k.land(atom(), k.eq(star(k, f, 0), k.F(f, 1))));
    Var* i = k.local(f, "i", k.i32);
    const Var* p = f->param_var(0);
    const Stmt* write = k.store(k.pn(p), k.add(rd(k, p), N(k, "1")));
    at = write->loc;
    std::vector<const Stmt*> body = {k.decl(i, N(k, "0"))};
    if (!reads_only) body.push_back(k.store(k.pn(p), N(k, "0")));
    body.push_back(k.whl(k.lt(k.v(i), k.pv(f, 1)), {k.cl(k.land(k.range(k.v(i), k.i32, "0", "100"), k.land(k.le(k.v(i), k.pv(f, 1)), k.eq(rd(k, p), k.v(i)))))},
                         {k.cl(k.sub(k.pv(f, 1), k.v(i)))}, k.blk({write, k.asg(i, k.add(k.v(i), N(k, "1")))})));
    f->body = k.blk(body);
    k.done(f);
    return at;
  };
  lc("mem_loop_param_place", "fill(p, n) with owns(*p): the parameter's place is the loop's, written in the loop with its value in the invariant", [=](Kit& k) { fill(k, false); });
  lc("mem_loop_param_place_reads_write", "the same with reads(*p): the place is held at less than write, the write in the loop has no permission", [=](Kit& k) { k.fail("permission", fill(k, true)); });
  // void drop(int *p, int n) requires owns(*p) && 0 <= n <= 100 { i = 0; while (i < n) { dispose(p); i += 1; } }
  const auto param_dispose = [=](Kit& k, bool consume) -> SrcLoc {
    SrcLoc at;
    Function* dispose = make_dispose(k);
    Function* f = k.fn("drop", k.vd, {{"p", PT(k)}, {"n", k.i32}});
    k.req(f, k.land(owns(k, f, 0, 0), k.range(k.F(f, 1), k.i32, "0", "100")));
    Var* i = k.local(f, "i", k.i32);
    const Var* p = f->param_var(0);
    std::vector<const Stmt*> inner;
    if (consume) inner.push_back(k.es(k.call(dispose, {k.pn(p)})));
    inner.push_back(k.asg(i, k.add(k.v(i), N(k, "1"))));
    const Stmt* body = k.blkc(inner);
    at = body->end_loc;
    f->body = k.blk({k.decl(i, N(k, "0")),
                     k.whl(k.lt(k.v(i), k.pv(f, 1)), {k.cl(k.range(k.v(i), k.i32, "0", "100")), k.cl(k.le(k.v(i), k.pv(f, 1)))}, {k.cl(k.sub(k.pv(f, 1), k.v(i)))}, body)});
    k.done(f);
    return at;
  };
  lc("mem_loop_param_untouched", "a loop that does not touch the parameter's place keeps it: nothing is consumed", [=](Kit& k) { param_dispose(k, false); });
  lc("mem_loop_param_dispose", "dispose(p) in a loop: the iteration ends without the permission to *p it began with, at the body's closing brace", [=](Kit& k) {
    k.fail_only("permission", param_dispose(k, true), 2, 0);  // of the 2 rows of the check (value and flag, then the liveness cell), the first fails
  });

  // ---- item 3: the values a loop keeps and the ones a write havocs ----
  // int x = 7; int y = 0; int *p = &x; int *q = &y; while (i > 0) { [y = i; | *q = i;] i -= 1; } return *p;   ensures r == 7
  const auto other_cell = [=](Kit& k, bool through_pointer) -> SrcLoc {
    SrcLoc at;
    Function* f = k.fn("f", k.i32, {{"n", k.i32}});
    k.req(f, k.range(k.F(f, 0), k.i32, "0", "100"));
    k.ens(f, k.eq_c(k.R(f), k.i32, "7"));
    Var* x = k.cell(f, "x", k.i32);
    Var* y = k.cell(f, "y", k.i32);
    Var* p = k.local(f, "p", PT(k));
    Var* q = k.local(f, "q", PT(k));
    Var* i = k.local(f, "i", k.i32);
    const Stmt* r = k.ret(rd(k, p));
    at = r->loc;
    const Stmt* write = through_pointer ? k.store(k.pn(q), k.v(i)) : k.asg(y, k.v(i));
    f->body = k.blkc({k.decl(x, N(k, "7")), k.decl(y, N(k, "0")), k.decl(p, k.addr(x)), k.decl(q, k.addr(y)), k.decl(i, k.pv(f, 0)),
                      count_down(k, i, {}, k.blk({write, step(k, i)})), r});
    k.done(f);
    return at;
  };
  lc("mem_loop_write_other_cell_by_name", "y = i in a loop does not reach x: x keeps its value", [=](Kit& k) { other_cell(k, false); });
  lc("mem_loop_write_through_pointer_havocs", "*q = i: a store through a pointer to an int havocs every int place, so x loses 7 (the owner's rule)", [=](Kit& k) { k.fail("postcondition", other_cell(k, true)); });
  // while (i > 0) { [peek(&x); | poke(&x);] i -= 1; } return x;   ensures r == 7
  const auto callee_value = [=](Kit& k, bool owning) -> SrcLoc {
    SrcLoc at;
    Function* callee = owning ? make_poke(k) : make_peek(k);
    cell_fn(k, "7", [&](Kit& k, Function*, Var* x, Var*, Var* i) {
      const Stmt* r = k.ret(k.v(x));
      at = r->loc;
      return std::vector<const Stmt*>{count_down(k, i, {}, k.blk({k.es(k.call(callee, {k.addr(x)})), step(k, i)})), r};
    }, "7");
    return at;
  };
  lc("mem_loop_reads_callee_keeps_value", "a callee with reads(*p) does not reach the cell's value: x keeps 7 across the loop", [=](Kit& k) { callee_value(k, false); });
  lc("mem_loop_owns_callee_havocs", "a callee whose contract owns an int cell havocs every int place: x loses 7", [=](Kit& k) { k.fail("postcondition", callee_value(k, true)); });

  // ---- item 6: a cell in an invariant and in a measure, with the framing ----
  // int x = 0; [dispose(&x);] while (i > 0) invariant x == <v> { i -= 1; } return 0;
  const auto cell_invariant = [=](Kit& k, int mode) -> SrcLoc {  // 0: x == 0, 1: x == 1 (wrong on entry), 2: x consumed before the loop (unframed)
    SrcLoc at;
    Function* dispose = make_dispose(k);
    cell_fn(k, "0", [&](Kit& k, Function*, Var* x, Var* p, Var* i) {
      const Expr* read = k.v(x);  // the framing row (mode 2) is at the read; in C source the read and its clause share a line
      Clause c = k.cl(k.eq(read, N(k, mode == 1 ? "1" : "0")));
      at = mode == 2 ? read->loc : c.loc;
      std::vector<const Stmt*> out;
      if (mode == 2) out.push_back(k.es(k.call(dispose, {k.pn(p)})));
      out.push_back(k.whl(k.gt(k.v(i), N(k, "0")), {k.cl(k.range(k.v(i), k.i32, "0", "100")), c}, {k.cl(k.v(i))}, k.blk({step(k, i)})));
      out.push_back(k.ret(N(k, "0")));
      return out;
    }, mode == 1 ? nullptr : "0");
    return at;
  };
  lc("mem_loop_cell_in_invariant", "a cell in a loop invariant: a fact about the value the place keeps", [=](Kit& k) { cell_invariant(k, 0); });
  lc("mem_loop_cell_in_invariant_entry", "x == 1 does not hold on entry", [=](Kit& k) { k.fail("invariant-entry", cell_invariant(k, 1)); });
  lc("mem_loop_cell_in_invariant_unframed", "the invariant reads x, whose object a callee kept before the loop: permission at the clause", [=](Kit& k) { k.fail_only("permission", cell_invariant(k, 2), 2, 0); });  // of the entry row and the back-edge row, the entry row fails
  // int k = n; int *p = &k; while (k > 0) invariant 0 <= k <= 100 decreases k { *p -= <0 | 1>; } return k;
  const auto cell_measure = [=](Kit& k, bool decreasing) -> SrcLoc {
    SrcLoc at;
    Function* f = k.fn("f", k.i32, {{"n", k.i32}});
    k.req(f, k.range(k.F(f, 0), k.i32, "0", "100"));
    k.ens(f, k.eq_c(k.R(f), k.i32, "0"));
    Var* c = k.cell(f, "k", k.i32);
    Var* p = k.local(f, "p", PT(k));
    Clause m = k.cl(k.v(c));
    at = m.loc;
    f->body = k.blkc({k.decl(c, k.pv(f, 0)), k.decl(p, k.addr(c)),
                      k.whl(k.gt(k.v(c), N(k, "0")), {k.cl(k.range(k.v(c), k.i32, "0", "100"))}, {m},
                            k.blk({k.cstore(k.pn(p), BinaryOp::Sub, N(k, decreasing ? "1" : "0"))})),
                      k.ret(k.v(c))});
    k.done(f);
    return at;
  };
  lc("mem_loop_cell_in_measure", "a cell in a loop measure and condition: DECREASES(k) with *p -= 1", [=](Kit& k) { cell_measure(k, true); });
  lc("mem_loop_cell_in_measure_wrong", "*p -= 0: the measure does not decrease", [=](Kit& k) { k.fail("termination", cell_measure(k, false)); });
  // int k = 5; int *p = &k; dispose(p); while (i < n) invariant ... decreases n - i + k { i += 1; }
  lc("mem_loop_cell_in_measure_unframed", "the measure reads k, whose object a callee kept before the loop: permission at the clause", [=](Kit& k) {
    Function* dispose = make_dispose(k);
    Function* f = k.fn("f", k.i32, {{"n", k.i32}});
    k.req(f, k.range(k.F(f, 0), k.i32, "0", "100"));
    Var* c = k.cell(f, "k", k.i32);
    Var* p = k.local(f, "p", PT(k));
    Var* i = k.local(f, "i", k.i32);
    const Expr* read = k.v(c);  // the framing row is at the read (in C source it shares a line with the decreases clause)
    Clause m = k.cl(k.add(k.sub(k.pv(f, 0), k.v(i)), read));
    f->body = k.blkc({k.decl(c, N(k, "5")), k.decl(p, k.addr(c)), k.es(k.call(dispose, {k.pn(p)})), k.decl(i, N(k, "0")),
                      k.whl(k.lt(k.v(i), k.pv(f, 0)), {k.cl(k.range(k.v(i), k.i32, "0", "100")), k.cl(k.le(k.v(i), k.pv(f, 0)))}, {m},
                            k.blk({k.asg(i, k.add(k.v(i), N(k, "1")))})),
                      k.ret(k.v(i))});
    k.done(f);
    k.fail_only("permission", read->loc, 2, 0);  // the entry row fails (the back-edge row sits on the body path that the entry failure does not reach)
  });

  // ---- item 2: a condition that reads memory (top-test shape), its checks as obligations at the condition's read ----
  // int x = 0; int *p = &x; while (*p < n) invariant 0 <= x <= n decreases n - x { *p += 1; } return x;
  lc("mem_loop_condition_deref", "while (*p < n) { *p += 1; }: the condition's checks run at the top of the body", [=](Kit& k) {
    Function* f = k.fn("f", k.i32, {{"n", k.i32}});
    k.req(f, k.range(k.F(f, 0), k.i32, "0", "100"));
    k.ens(f, k.eq(k.R(f), k.F(f, 0)));
    Var* x = k.cell(f, "x", k.i32);
    Var* p = k.local(f, "p", PT(k));
    f->body = k.blkc({k.decl(x, N(k, "0")), k.decl(p, k.addr(x)),
                      k.whl(k.lt(rd(k, p), k.pv(f, 0)), {k.cl(k.land(k.ge_c(k.v(x), k.i32, "0"), k.le(k.v(x), k.pv(f, 0))))}, {k.cl(k.sub(k.pv(f, 0), k.v(x)))},
                            k.blk({k.cstore(k.pn(p), BinaryOp::Add, N(k, "1"))})),
                      k.ret(k.v(x))});
    k.done(f);
  });
  const auto bad_condition = [=](Kit& k, int mode) -> SrcLoc {  // 0: p is null, 1: x never written
    Function* f = k.fn("f", k.i32, {{"n", k.i32}});
    k.req(f, k.range(k.F(f, 0), k.i32, "0", "100"));
    Var* x = k.cell(f, "x", k.i32);
    Var* p = k.local(f, "p", PT(k));
    Var* i = k.local(f, "i", k.i32);
    const Expr* read = mode == 0 ? rd(k, p) : k.v(x);
    f->body = k.blkc({k.decl(x, nullptr), k.decl(p, mode == 0 ? k.nul(PT(k)) : k.addr(x)), k.decl(i, N(k, "0")),
                      k.whl(k.land(k.lt(k.v(i), k.pv(f, 0)), k.lt(read, k.pv(f, 0))), {k.cl(k.range(k.v(i), k.i32, "0", "100")), k.cl(k.le(k.v(i), k.pv(f, 0)))}, {k.cl(k.sub(k.pv(f, 0), k.v(i)))},
                            k.blk({k.asg(i, k.add(k.v(i), N(k, "1")))})),
                      k.ret(k.v(i))});
    k.done(f);
    return read->loc;
  };
  lc("mem_loop_condition_null", "while (*p < n) with p == NULL: the condition's own check is an obligation", [=](Kit& k) { k.fail("null-dereference", bad_condition(k, 0)); });
  lc("mem_loop_condition_uninitialized", "while (x < n) with x never written", [=](Kit& k) { k.fail("uninitialized", bad_condition(k, 1)); });

  // ---- item 7: a body cell, continue and break ----
  // while (i < n) { int x = 1; int *p = &x; i += 1; if (i == 7) { total += *p; [dispose(p);] continue; } total += *p; } return total;   ensures r == n
  const auto body_cell = [=](Kit& k, int mode, SrcLoc* at) {  // 0: fine, 1: consumed before the continue, 2: consumed before the closing brace
    Function* dispose = make_dispose(k);
    Function* f = k.fn("f", k.i32, {{"n", k.i32}});
    k.req(f, k.range(k.F(f, 0), k.i32, "0", "100"));
    k.ens(f, k.eq(k.R(f), k.F(f, 0)));
    Var* total = k.local(f, "total", k.i32);
    Var* i = k.local(f, "i", k.i32);
    Var* x = k.cell(f, "x", k.i32);
    Var* p = k.local(f, "p", PT(k));
    const int id = k.jid();
    const Stmt* cont = k.cnt(id);
    std::vector<const Stmt*> then = {k.asg(total, k.add(k.v(total), rd(k, p)))};
    if (mode == 1) then.push_back(k.es(k.call(dispose, {k.pn(p)})));
    then.push_back(cont);
    std::vector<const Stmt*> inner = {k.decl(x, N(k, "1")), k.decl(p, k.addr(x)), k.asg(i, k.add(k.v(i), N(k, "1"))),
                                      k.iff(k.eq(k.v(i), N(k, "7")), k.blk(then)), k.asg(total, k.add(k.v(total), rd(k, p)))};
    if (mode == 2) inner.push_back(k.es(k.call(dispose, {k.pn(p)})));
    const Stmt* body = k.blkc(inner);
    *at = mode == 1 ? cont->loc : body->end_loc;
    f->body = k.blk({k.decl(total, N(k, "0")), k.decl(i, N(k, "0")),
                     k.loop(k.lt(k.v(i), k.pv(f, 0)), {k.cl(k.land(k.range(k.v(i), k.i32, "0", "100"), k.land(k.le(k.v(i), k.pv(f, 0)), k.eq(k.v(total), k.v(i)))))},
                            {k.cl(k.sub(k.pv(f, 0), k.v(i)))}, body, nullptr, LF::While, id),
                     k.ret(k.v(total))});
    k.done(f);
  };
  lc("mem_loop_body_cell_continue", "a cell declared in the loop body is a new block per iteration; a continue leaves it", [=](Kit& k) { SrcLoc l; body_cell(k, 0, &l); });
  lc("mem_loop_body_cell_continue_consumed", "the callee kept the cell before a continue: the scope exit at the continue fails", [=](Kit& k) { SrcLoc l; body_cell(k, 1, &l); k.fail("lifetime", l); });
  lc("mem_loop_body_cell_brace_consumed", "the callee kept the cell: the scope exit at the body's closing brace fails", [=](Kit& k) { SrcLoc l; body_cell(k, 2, &l); k.fail("lifetime", l); });
  // while (i < n) { int x = i; int *p = &x; if (*p == 5) { [dispose(p);] break; } i += 1; } return i;   ensures r <= n && r <= 5
  const auto body_break = [=](Kit& k, bool consumed) -> SrcLoc {
    Function* dispose = make_dispose(k);
    Function* f = k.fn("f", k.i32, {{"n", k.i32}});
    k.req(f, k.range(k.F(f, 0), k.i32, "0", "100"));
    k.ens(f, k.land(k.le(k.R(f), k.F(f, 0)), k.le_c(k.R(f), k.i32, "5")));
    Var* i = k.local(f, "i", k.i32);
    Var* x = k.cell(f, "x", k.i32);
    Var* p = k.local(f, "p", PT(k));
    const int id = k.jid();
    const Stmt* brk = k.brk(id);
    std::vector<const Stmt*> then;
    if (consumed) then.push_back(k.es(k.call(dispose, {k.pn(p)})));
    then.push_back(brk);
    const Stmt* body = k.blkc({k.decl(x, k.v(i)), k.decl(p, k.addr(x)), k.iff(k.eq(rd(k, p), N(k, "5")), k.blk(then)), k.asg(i, k.add(k.v(i), N(k, "1")))});
    f->body = k.blk({k.decl(i, N(k, "0")),
                     k.loop(k.lt(k.v(i), k.pv(f, 0)), {k.cl(k.land(k.range(k.v(i), k.i32, "0", "100"), k.land(k.le(k.v(i), k.pv(f, 0)), k.le_c(k.v(i), k.i32, "5"))))},
                            {k.cl(k.sub(k.pv(f, 0), k.v(i)))}, body, nullptr, LF::While, id),
                     k.ret(k.v(i))});
    k.done(f);
    return brk->loc;
  };
  lc("mem_loop_body_cell_break", "a break out of a loop body that declares a cell: the scope exit of the cell is at the break", [=](Kit& k) { body_break(k, false); });
  lc("mem_loop_body_cell_break_consumed", "the callee kept the cell, so the break's scope exit fails at the break", [=](Kit& k) { k.fail("lifetime", body_break(k, true)); });

  // ---- item 3 (S28 F3, audit B4): a return inside a loop that names no memory ----
  // int find(int *p, int n) requires owns(*p) && 0 <= n <= 100 ensures owns(*p) && 0 <= r <= n { i = 0; while (i < n) { if (i == 5) { [dispose(p);] return i; } i += 1; } return n; }
  const auto return_owns = [=](Kit& k, bool consumed) -> SrcLoc {
    Function* dispose = make_dispose(k);
    Function* f = k.fn("find", k.i32, {{"p", PT(k)}, {"n", k.i32}});
    k.req(f, k.land(owns(k, f, 0, 0), k.range(k.F(f, 1), k.i32, "0", "100")));
    k.ens(f, k.land(owns(k, f, 0, 0), k.land(k.ge_c(k.R(f), k.i32, "0"), k.le(k.R(f), k.F(f, 1)))));
    Var* i = k.local(f, "i", k.i32);
    const Var* p = f->param_var(0);
    const Stmt* r = k.ret(k.v(i));
    std::vector<const Stmt*> then;
    if (consumed) then.push_back(k.es(k.call(dispose, {k.pn(p)})));
    then.push_back(r);
    f->body = k.blk({k.decl(i, N(k, "0")),
                     k.whl(k.lt(k.v(i), k.pv(f, 1)), {k.cl(k.range(k.v(i), k.i32, "0", "100")), k.cl(k.le(k.v(i), k.pv(f, 1)))}, {k.cl(k.sub(k.pv(f, 1), k.v(i)))},
                           k.blk({k.iff(k.eq(k.v(i), N(k, "5")), k.blk(then)), k.asg(i, k.add(k.v(i), N(k, "1")))})),
                     k.ret(k.pv(f, 1))});
    k.done(f);
    return r->loc;
  };
  lc("mem_loop_return_owns", "a return inside an int-only loop of a function whose contract owns *p: the loop holds the place, the return gives it back", [=](Kit& k) { return_owns(k, false); });
  lc("mem_loop_return_owns_consumed", "the callee kept *p before the return: the return does not give back what its ensures promises", [=](Kit& k) {
    k.fail_only("permission", return_owns(k, true), 6, 1);  // of the 6 rows of the return check, the value: perm(p.v_int) == write
  });
  // int pass(int a, int n) { int *pa = &a; i = 0; while (i < n) { if (i == 5) { [dispose(pa);] return i; } i += 1; } return *pa; }   (a &param cell)
  const auto return_param_cell = [=](Kit& k, bool consumed) -> SrcLoc {
    Function* dispose = make_dispose(k);
    Function* f = k.fn("pass", k.i32, {{"a", k.i32}, {"n", k.i32}});
    k.req(f, k.land(k.range(k.F(f, 0), k.i32, "0", "100"), k.range(k.F(f, 1), k.i32, "0", "100")));
    if (!consumed) k.ens(f, k.range(k.R(f), k.i32, "0", "100"));  // a callee that owns an int cell havocs the cell: the twin states nothing about it
    f->mark_address_taken(f->param_var(0));
    Var* pa = k.local(f, "pa", PT(k));
    Var* i = k.local(f, "i", k.i32);
    const Stmt* r = k.ret(k.v(i));
    std::vector<const Stmt*> then;
    if (consumed) then.push_back(k.es(k.call(dispose, {k.pn(pa)})));
    then.push_back(r);
    f->body = k.blkc({k.decl(pa, k.addr(f->param_var(0))), k.decl(i, N(k, "0")),
                      k.whl(k.lt(k.v(i), k.pv(f, 1)), {k.cl(k.range(k.v(i), k.i32, "0", "100")), k.cl(k.le(k.v(i), k.pv(f, 1)))}, {k.cl(k.sub(k.pv(f, 1), k.v(i)))},
                            k.blk({k.iff(k.eq(k.v(i), N(k, "5")), k.blk(then)), k.asg(i, k.add(k.v(i), N(k, "1")))})),
                      k.ret(rd(k, pa))});
    k.done(f);
    return r->loc;
  };
  lc("mem_loop_return_param_cell", "a return inside a loop that names no memory while a &param cell is live: the loop holds the cell", [=](Kit& k) { return_param_cell(k, false); });
  lc("mem_loop_return_param_cell_consumed", "the callee kept the parameter's copy, so the scope exit at the return fails", [=](Kit& k) { k.fail("lifetime", return_param_cell(k, true)); });
  // while (i < n) { if (i == 5) { return <3 | 4>; } i += 1; } int x = 3; int *p = &x; return *p;   ensures r == 3   (x is allocated at the block's entry)
  const auto declared_after = [=](Kit& k, bool wrong) -> SrcLoc {
    SrcLoc at;
    Function* f = k.fn("f", k.i32, {{"n", k.i32}});
    k.req(f, k.range(k.F(f, 0), k.i32, "0", "100"));
    k.ens(f, k.eq_c(k.R(f), k.i32, "3"));
    Var* i = k.local(f, "i", k.i32);
    Var* x = k.cell(f, "x", k.i32);
    Var* p = k.local(f, "p", PT(k));
    const Stmt* r = k.ret(N(k, wrong ? "4" : "3"));
    at = r->loc;
    f->body = k.blkc({k.decl(i, N(k, "0")),
                      k.whl(k.lt(k.v(i), k.pv(f, 0)), {k.cl(k.range(k.v(i), k.i32, "0", "100")), k.cl(k.le(k.v(i), k.pv(f, 0)))}, {k.cl(k.sub(k.pv(f, 0), k.v(i)))},
                            k.blk({k.iff(k.eq(k.v(i), N(k, "5")), k.blk({r})), k.asg(i, k.add(k.v(i), N(k, "1")))})),
                      k.decl(x, N(k, "3")), k.decl(p, k.addr(x)), k.ret(rd(k, p))});
    k.done(f);
    return at;
  };
  lc("mem_loop_cell_declared_after", "a cell declared after the loop in the enclosing block is allocated at its entry: the loop holds it, a return inside the loop exits it", [=](Kit& k) { declared_after(k, false); });
  lc("mem_loop_cell_declared_after_wrong", "the return inside the loop is checked: it returns 4, the contract says 3", [=](Kit& k) { k.fail("postcondition", declared_after(k, true)); });

  // ---- item 3 and 6: a pointer the loop assigns ----
  // int x = 1; int y = 2; int *p = &x; while (i > 0) invariant [p == &x || p == &y] { s += *p; p = (i % 2 == 0) ? &y : &x; i -= 1; } return *p;   ensures r == 1 || r == 2
  const auto pick = [=](Kit& k, int mode, std::vector<SrcLoc>* at) {  // mode 0: no clause on p, 1: p == &x || p == &y, 2: p == &x
    Function* f = k.fn("f", k.i32, {{"n", k.i32}});
    k.req(f, k.range(k.F(f, 0), k.i32, "0", "100"));
    k.ens(f, k.lor(k.eq_c(k.R(f), k.i32, "1"), k.eq_c(k.R(f), k.i32, "2")));
    Var* x = k.cell(f, "x", k.i32);
    Var* y = k.cell(f, "y", k.i32);
    Var* p = k.local(f, "p", PT(k));
    Var* s = k.local(f, "s", k.i32);
    Var* i = k.local(f, "i", k.i32);
    const Expr* in_body = rd(k, p);
    const Expr* after = rd(k, p);
    at->push_back(in_body->loc);
    at->push_back(after->loc);
    std::vector<Clause> extra = {k.cl(k.land(k.ge_c(k.v(s), k.i32, "0"), k.le(k.v(s), k.mul(N(k, "2"), k.sub(k.pv(f, 0), k.v(i)))))) };
    // s <= 2 * (n - i): the sum so far (an invariant that holds by the steps below)
    if (mode == 1) extra.push_back(k.cl(k.lor(k.eq(k.pn(p), k.addr(x)), k.eq(k.pn(p), k.addr(y)))));
    if (mode == 2) {
      extra.push_back(k.cl(k.eq(k.pn(p), k.addr(x))));
      at->push_back(extra.back().loc);
    }
    f->body = k.blkc({k.decl(x, N(k, "1")), k.decl(y, N(k, "2")), k.decl(p, k.addr(x)), k.decl(s, N(k, "0")), k.decl(i, k.pv(f, 0)),
                      count_down(k, i, extra,
                                 k.blk({k.asg(s, k.add(k.v(s), in_body)), k.asg(p, k.cond(k.eq(k.rem(k.v(i), N(k, "2")), N(k, "0")), k.addr(y), k.addr(x))), step(k, i)})),
                      k.ret(after)});
    k.done(f);
  };
  lc("mem_loop_pick_pointer", "a pointer the loop assigns, used before the reassignment and after the loop, with p == &x || p == &y in the invariant", [=](Kit& k) { std::vector<SrcLoc> at; pick(k, 1, &at); });
  lc("mem_loop_pick_pointer_unconstrained", "without the invariant the pointer is any pointer at the loop head and after the loop: at each use a null-dereference and (the check assumed) no permission", [=](Kit& k) {
    std::vector<SrcLoc> at;
    pick(k, 0, &at);
    for (const SrcLoc& l : at) {
      k.fail("null-dereference", l);
      k.fail("permission", l);
    }
  });
  lc("mem_loop_pick_pointer_wrong_invariant", "p == &x alone is not preserved: the loop may assign &y", [=](Kit& k) {
    std::vector<SrcLoc> at;
    pick(k, 2, &at);
    k.fail("invariant-preserved", at[2]);
  });

  // ---- the other loop forms: for with a step, do, a loop with a continue path ----
  // while (i < n) { *p += 1; i += 1; } as a `for` (step i += 1) and as a `do` (n >= 1); the body may consume the cell first
  const auto form = [=](Kit& k, LF lf, bool consume) -> SrcLoc {
    Function* dispose = make_dispose(k);
    Function* f = k.fn("f", k.i32, {{"n", k.i32}});
    k.req(f, k.range(k.F(f, 0), k.i32, lf == LF::DoWhile ? "1" : "0", "100"));
    if (!consume) k.ens(f, k.eq(k.R(f), k.F(f, 0)));
    Var* x = k.cell(f, "x", k.i32);
    Var* p = k.local(f, "p", PT(k));
    Var* i = k.local(f, "i", k.i32);
    std::vector<const Stmt*> inner;
    if (!consume) inner.push_back(k.cstore(k.pn(p), BinaryOp::Add, N(k, "1")));
    if (lf == LF::DoWhile) inner.push_back(k.asg(i, k.add(k.v(i), N(k, "1"))));
    // the consuming body keeps the exit path whole: the cell is given away only when the loop goes on
    if (consume) inner.push_back(k.iff(k.lt(k.v(i), k.pv(f, 0)), k.blk({k.es(k.call(dispose, {k.pn(p)}))})));
    const Stmt* body = k.blkc(inner);
    const Stmt* stepper = lf == LF::While ? k.asg(i, k.add(k.v(i), N(k, "1"))) : nullptr;
    // a do loop's invariant holds at the top of each iteration, where its condition held (i < n): at the exit i == n
    std::vector<Clause> inv = {k.cl(k.range(k.v(i), k.i32, "0", "100")), k.cl(lf == LF::DoWhile ? k.lt(k.v(i), k.pv(f, 0)) : k.le(k.v(i), k.pv(f, 0)))};
    if (!consume) inv.push_back(k.cl(k.eq(k.v(x), k.v(i))));
    const Stmt* loop = k.loop(k.lt(k.v(i), k.pv(f, 0)), inv, {k.cl(k.sub(k.pv(f, 0), k.v(i)))}, body, stepper, lf, k.jid());
    f->body = k.blkc({k.decl(x, N(k, "0")), k.decl(p, k.addr(x)), k.decl(i, N(k, "0")), loop, k.ret(consume ? k.v(i) : k.v(x))});
    k.done(f);
    return body->end_loc;
  };
  lc("mem_loop_for_step", "a for loop (a step) over a cell: the back edge is after the step", [=](Kit& k) { form(k, LF::While, false); });
  lc("mem_loop_for_step_consumed", "the body consumes the cell: reported at the body's closing brace", [=](Kit& k) { k.fail("permission", form(k, LF::While, true)); });
  lc("mem_loop_do_form", "a do loop over a cell: the back edge is after the exit test", [=](Kit& k) { form(k, LF::DoWhile, false); });
  lc("mem_loop_do_form_consumed", "the body consumes the cell: reported at the body's closing brace", [=](Kit& k) { k.fail("permission", form(k, LF::DoWhile, true)); });

  // ---- item 3: parameters that may alias, a pointer that may be NULL ----
  // int sum(const int *a, const int *b, int n) requires reads(*a) && reads(*b) && 0 <= *a <= 10 && 0 <= *b <= <10 | any> ...
  const auto alias_sum = [=](Kit& k, bool bounded) -> SrcLoc {
    SrcLoc at;
    Function* f = k.fn("sum", k.i32, {{"a", PT(k)}, {"b", PT(k)}, {"n", k.i32}});
    k.req(f, k.land(k.land(reads(k, f, 0, 0), reads(k, f, 1, 1)),
                    k.land(k.land(k.range(star(k, f, 0), k.i32, "0", "10"), bounded ? k.range(star(k, f, 1), k.i32, "0", "10") : k.ge_c(star(k, f, 1), k.i32, "0")),
                           k.range(k.F(f, 2), k.i32, "0", "100"))));
    // the twin states only s >= 0 and r >= 0 (no upper bound), so the overflow is the one thing that fails
    k.ens(f, k.land(k.land(reads(k, f, 0, 0), reads(k, f, 1, 1)), bounded ? k.range(k.R(f), k.i32, "0", "2000") : k.ge_c(k.R(f), k.i32, "0")));
    Var* s = k.local(f, "s", k.i32);
    Var* i = k.local(f, "i", k.i32);
    const Var* a = f->param_var(0);
    const Var* b = f->param_var(1);
    const Expr* sum = k.add(rd(k, a), rd(k, b));
    const Expr* total = k.add(k.v(s), sum);
    at = total->loc;
    if (!bounded) k.fail("overflow", sum->loc);  // *a + *b may overflow too (the sum is checked before the addition to s)
    const Expr* s_bound = bounded ? k.land(k.ge_c(k.v(s), k.i32, "0"), k.le(k.v(s), k.mul(N(k, "20"), k.v(i)))) : k.ge_c(k.v(s), k.i32, "0");
    f->body = k.blk({k.decl(s, N(k, "0")), k.decl(i, N(k, "0")),
                     k.whl(k.lt(k.v(i), k.pv(f, 2)), {k.cl(k.land(k.range(k.v(i), k.i32, "0", "100"), k.land(k.le(k.v(i), k.pv(f, 2)), s_bound)))},
                           {k.cl(k.sub(k.pv(f, 2), k.v(i)))}, k.blk({k.asg(s, total), k.asg(i, k.add(k.v(i), N(k, "1")))})),
                     k.ret(k.v(s))});
    k.done(f);
    return at;
  };
  lc("mem_loop_reads_alias", "two reads parameters that may alias are two automatic places counted once: summed in a loop", [=](Kit& k) { alias_sum(k, true); });
  lc("mem_loop_reads_alias_overflow", "without the upper bound on *b the sum may overflow", [=](Kit& k) { k.fail("overflow", alias_sum(k, false)); });
  // int count(int *p, int n) requires (p == NULL || owns(*p)) ... { while (i < n) { if (p != NULL && *p > 0) c += 1; i += 1; } return c; }
  const auto null_or_owns = [=](Kit& k, bool guarded) -> SrcLoc {
    SrcLoc at;
    Function* f = k.fn("count", k.i32, {{"p", PT(k)}, {"n", k.i32}});
    const auto is_null = [&]() { return k.eq(FP(k, f, 0), k.nul(PT(k))); };
    k.req(f, k.land(k.lor(is_null(), owns(k, f, 0, 0)), k.range(k.F(f, 1), k.i32, "0", "100")));
    k.ens(f, k.land(k.lor(is_null(), owns(k, f, 0, 0)), k.land(k.ge_c(k.R(f), k.i32, "0"), k.le(k.R(f), k.F(f, 1)))));
    Var* c = k.local(f, "c", k.i32);
    Var* i = k.local(f, "i", k.i32);
    const Var* p = f->param_var(0);
    const Expr* d = rd(k, p);
    at = d->loc;
    const Expr* test = guarded ? k.land(k.ne(k.pn(p), k.nul(PT(k))), k.gt(d, N(k, "0"))) : k.gt(d, N(k, "0"));
    f->body = k.blk({k.decl(c, N(k, "0")), k.decl(i, N(k, "0")),
                     k.whl(k.lt(k.v(i), k.pv(f, 1)),
                           {k.cl(k.land(k.range(k.v(i), k.i32, "0", "100"), k.land(k.le(k.v(i), k.pv(f, 1)), k.land(k.ge_c(k.v(c), k.i32, "0"), k.le(k.v(c), k.v(i))))))},
                           {k.cl(k.sub(k.pv(f, 1), k.v(i)))},
                           k.blk({k.iff(test, k.blk({k.asg(c, k.add(k.v(c), N(k, "1")))})), k.asg(i, k.add(k.v(i), N(k, "1")))})),
                     k.ret(k.v(c))});
    k.done(f);
    return at;
  };
  lc("mem_loop_null_or_owns", "a p == NULL || owns(*p) parameter in a loop: the place is guarded by p != NULL, the dereference under p != NULL", [=](Kit& k) { null_or_owns(k, true); });
  lc("mem_loop_null_or_owns_unguarded", "the loop dereferences p without the test", [=](Kit& k) { k.fail("null-dereference", null_or_owns(k, false)); });

  // ---- a conditional atom (audit round 2, B1) ----
  // int f(int *p, int c) requires c == 0 || owns(*p) ensures (c == 0 || owns(*p)) && r == <9 | 10> { int x = 0; int *q = &x; i = 0; while (i < 10) invariant i == 0 || x == i - 1 { *q = i; i += 1; } return x; }
  const auto conditional_owns = [=](Kit& k, bool wrong) -> SrcLoc {
    SrcLoc at;
    Function* f = k.fn("f", k.i32, {{"p", PT(k)}, {"c", k.i32}});
    const auto guarded = [&]() { return k.lor(k.eq_c(k.F(f, 1), k.i32, "0"), owns(k, f, 0, 0)); };
    k.req(f, guarded());
    k.ens(f, k.land(guarded(), k.eq_c(k.R(f), k.i32, wrong ? "10" : "9")));
    Var* x = k.cell(f, "x", k.i32);
    Var* q = k.local(f, "q", PT(k));
    Var* i = k.local(f, "i", k.i32);
    const Stmt* r = k.ret(k.v(x));
    at = r->loc;
    f->body = k.blkc({k.decl(x, N(k, "0")), k.decl(q, k.addr(x)), k.decl(i, N(k, "0")),
                      k.whl(k.lt(k.v(i), N(k, "10")),
                            {k.cl(k.range(k.v(i), k.i32, "0", "10")), k.cl(k.lor(k.eq_c(k.v(i), k.i32, "0"), k.eq(k.v(x), k.sub(k.v(i), N(k, "1")))))},
                            {k.cl(k.sub(N(k, "10"), k.v(i)))}, k.blk({k.store(k.pn(q), k.v(i)), k.asg(i, k.add(k.v(i), N(k, "1")))})),
                      r});
    k.done(f);
    return at;
  };
  lc("mem_loop_conditional_owns", "a parameter with c == 0 || owns(*p) and a cell written through a pointer in a loop: the conditional atom's place is guarded", [=](Kit& k) { conditional_owns(k, false); });
  lc("mem_loop_conditional_owns_wrong", "the cell holds 9 after the loop, not 10", [=](Kit& k) { k.fail("postcondition", conditional_owns(k, true)); });

  // ---- the flag (S28 F2): a cell first initialized in the loop ----
  // int x; int *p = &x; while (i < n) [invariant i == 0 || x == i] { *p = i + 1; i += 1; } return *p;
  const auto init_in_loop = [=](Kit& k, bool with_value) -> SrcLoc {
    SrcLoc at;
    Function* f = k.fn("f", k.i32, {{"n", k.i32}});
    k.req(f, k.range(k.F(f, 0), k.i32, "1", "100"));
    if (with_value) k.ens(f, k.eq(k.R(f), k.F(f, 0)));
    Var* x = k.cell(f, "x", k.i32);
    Var* p = k.local(f, "p", PT(k));
    Var* i = k.local(f, "i", k.i32);
    std::vector<Clause> inv = {k.cl(k.range(k.v(i), k.i32, "0", "100")), k.cl(k.le(k.v(i), k.pv(f, 0)))};
    if (with_value) inv.push_back(k.cl(k.lor(k.eq_c(k.v(i), k.i32, "0"), k.eq(k.v(x), k.v(i)))));
    const Expr* d = rd(k, p);
    at = d->loc;
    f->body = k.blkc({k.decl(x, nullptr), k.decl(p, k.addr(x)), k.decl(i, N(k, "0")),
                      k.whl(k.lt(k.v(i), k.pv(f, 0)), inv, {k.cl(k.sub(k.pv(f, 0), k.v(i)))},
                            k.blk({k.store(k.pn(p), k.add(k.v(i), N(k, "1"))), k.asg(i, k.add(k.v(i), N(k, "1")))})),
                      k.ret(d)});
    k.done(f);
    return at;
  };
  lc("mem_loop_init_in_loop", "a cell first initialized in the loop, read after it, the invariant states its value", [=](Kit& k) { init_in_loop(k, true); });
  lc("mem_loop_init_in_loop_read", "without the value in the invariant the flag is only known to stay true: the read after the loop is uninitialized", [=](Kit& k) { k.fail("uninitialized", init_in_loop(k, false)); });
}

void define_loops_and_jumps_rejections() {
  auto PT = [](Kit& k) { return k.ptr(k.i32); };
  auto N = [](Kit& k, const char* v) { return k.num(k.i32, v); };
  // an atom in the invariant of a do ... while (0): straight-line code, asserted like a check (never lifted)
  rc("mem_rej_atom_in_do_once_invariant", "a permission atom outside a contract", [=](Kit& k) {
    Function* f = k.fn("f", k.vd, {{"p", PT(k)}});
    k.req(f, k.b.atom(AtomKind::Owns, k.b.text(k.F(f, 0), "p"), 0, k.at()));
    const Expr* a = k.b.atom(AtomKind::Owns, k.b.text(k.F(f, 0), "p"), 0, k.at());
    f->body = k.blk({k.loop(k.no(), {k.cl(a)}, {}, k.blk({}), nullptr, LoopForm::DoOnce, k.jid())});
    k.done(f);
  });
  // an atom in a loop measure (never lifted): the check_program backstop of the encoder's rejection
  rc("mem_rej_atom_in_loop_measure", "a permission atom outside a contract", [=](Kit& k) {
    Function* f = k.fn("f", k.vd, {{"p", PT(k)}});
    k.req(f, k.b.atom(AtomKind::Owns, k.b.text(k.F(f, 0), "p"), 0, k.at()));
    Var* i = k.local(f, "i", k.i32);
    const Expr* a = k.b.atom(AtomKind::Owns, k.b.text(k.F(f, 0), "p"), 0, k.at());
    f->body = k.blk({k.decl(i, N(k, "1")), k.whl(k.gt(k.v(i), N(k, "0")), {}, {k.cl(k.cond(a, k.v(i), N(k, "0")))}, k.blk({k.asg(i, k.sub(k.v(i), N(k, "1")))}))});
    k.done(f);
  });
  // &y in a loop invariant where y is not marked address-taken (the code never takes it)
  rc("mem_rej_address_of_unmarked_in_invariant", "which is not marked address-taken", [=](Kit& k) {
    Function* f = k.fn("f", k.vd, {{"n", k.i32}});
    Var* y = k.local(f, "y", k.i32);
    Var* i = k.local(f, "i", k.i32);
    Var* p = k.local(f, "p", PT(k));
    Var* x = k.cell(f, "x", k.i32);
    f->body = k.blkc({k.decl(x, N(k, "0")), k.decl(y, N(k, "0")), k.decl(p, k.addr(x)), k.decl(i, k.pv(f, 0)),
                      k.whl(k.gt(k.v(i), N(k, "0")), {k.cl(k.ne(k.pn(p), k.addr(y)))}, {k.cl(k.v(i))}, k.blk({k.asg(i, k.sub(k.v(i), N(k, "1")))}))});
    k.done(f);
  });
}

}  // namespace

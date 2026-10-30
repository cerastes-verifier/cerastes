// (included by selftest.cpp) Sequencing and placement over memory (decision 0039, M4.5b1c2): writes to cells inside expressions (an AssignE on a
// cell, a StoreE plain, compound and with `yields_old`), sequencing on objects (item 3's path-precise asserts and the static rejections), the calls
// that C does not order with a cell access or with each other (item 4) and the order they must keep, B2's sets (a call that never returns must not
// hide a check C may evaluate first), conditional ownership and its partners by path (item 7, question 72), a function's measure over memory
// (item 8: its framing and the explicit termination check), the two fail-closed hooks of item 4, and the rejections that stay.
// PRE-REGISTERED from the record (item numbers below), before lowering ran these cases: each expectation names the category and the VIR node whose
// position the record gives (the later-created access or call for `unsequenced`, the store for `null-dereference` and `permission`, the recursive
// call for `termination`, the atom or the read for item 7 and item 8's framing), so the cases test the position map as well.  Each rule has a case
// that verifies and a twin that fails for one reason (invariant 2).  The two `mem_rej_cell_*_as_value` cases of b1a (selftest_cases7.inc, item 9 of
// decision 0032) are the first two lowering cases here (pre-approved flips).
namespace {

void define_sequencing() {
  auto PT = [](Kit& k) { return k.ptr(k.i32); };
  auto N = [](Kit& k, const char* v) { return k.num(k.i32, v); };
  auto FP = [](Kit& k, Function* f, int i) { return k.b.text(k.F(f, i), f->params[static_cast<size_t>(i)].name); };
  auto owns = [=](Kit& k, Function* f, int i, int amount) { return k.b.atom(AtomKind::Owns, FP(k, f, i), amount, k.at()); };
  auto reads = [=](Kit& k, Function* f, int i, int amount) { return k.b.atom(AtomKind::Reads, FP(k, f, i), amount, k.at()); };
  auto live = [=](Kit& k, Function* f, int i, int amount) { return k.b.atom(AtomKind::Live, FP(k, f, i), amount, k.at()); };
  auto star = [=](Kit& k, Function* f, int i) { return k.deref(FP(k, f, i)); };  // `*p` in a contract
  auto old_star = [=](Kit& k, Function* f, int i) { return k.old(star(k, f, i)); };
  // A cell `name` of `f`, declared with `v`.
  auto cell_init = [=](Kit& k, Function* f, const char* name, const char* v) {
    Var* x = k.cell(f, name, k.i32);
    return std::make_pair(x, k.decl(x, N(k, v)));
  };
  // The writes through a pointer used as a value (decision 0039 item 2).
  auto store_e = [](Kit& k, const Expr* addr, const Expr* value) { return k.b.store_expr(addr, value, k.at()); };
  auto cstore_e = [](Kit& k, const Expr* addr, BinaryOp op, const Expr* rhs) { return k.b.compound_store_expr(addr, op, rhs, k.at()); };
  auto idstore_e = [](Kit& k, const Expr* addr, bool increment, bool post) { return k.b.inc_dec_store_expr(addr, increment, post, k.at()); };

  // int bump(int *p) requires(owns(*p) && *p < 100) ensures(owns(*p) && *p == old(*p) + 1 && r == *p);   (trusted)
  const auto make_bump = [=](Kit& k) -> Function* {
    Function* d = k.fn("bump", k.i32, {{"p", PT(k)}});
    k.req(d, k.land(owns(k, d, 0, 0), k.lt(star(k, d, 0), N(k, "100"))));
    k.ens(d, k.land(k.land(owns(k, d, 0, 0), k.eq(star(k, d, 0), k.add(old_star(k, d, 0), N(k, "1")))), k.eq(k.R(d), star(k, d, 0))));
    k.trusted(d);
    return d;
  };
  // int peek(const int *p) requires(reads(*p)) ensures(reads(*p) && r == *p);   (trusted)
  const auto make_peek = [=](Kit& k) -> Function* {
    Function* d = k.fn("peek", k.i32, {{"p", PT(k)}});
    k.req(d, reads(k, d, 0, 0));
    k.ens(d, k.land(reads(k, d, 0, 0), k.eq(k.R(d), star(k, d, 0))));
    k.trusted(d);
    return d;
  };
  // int nr(void) ensures(false);   (trusted: a call that never returns)
  const auto make_nr = [=](Kit& k) -> Function* {
    Function* d = k.fn("nr", k.i32);
    k.ens(d, k.no());
    k.trusted(d);
    return d;
  };
  // int *first(int *p) requires(owns(*p)) ensures(r == p && owns(*p) && *p == old(*p));   (trusted)
  const auto make_first = [=](Kit& k) -> Function* {
    Function* d = k.fn("first", PT(k), {{"p", PT(k)}});
    k.req(d, owns(k, d, 0, 0));
    k.ens(d, k.land(k.land(k.eq(k.R(d), FP(k, d, 0)), owns(k, d, 0, 0)), k.eq(star(k, d, 0), old_star(k, d, 0))));
    k.trusted(d);
    return d;
  };

  // ---- item 2: an assignment to a cell used as a value (the flip of mem_rej_cell_assignment_as_value) ----
  // int f() { int x = 0; int y = (x = 1); check(y == 1 && x == 1); return y; }
  const auto asg_cell = [=](Kit& k, bool wrong) -> SrcLoc {
    Function* f = k.fn("f", k.i32);
    auto [x, dx] = cell_init(k, f, "x", "0");
    Var* y = k.local(f, "y", k.i32);
    const Stmt* dy = k.decl(y, k.asgE(x, N(k, "1")));
    const Stmt* c = k.chk(k.land(k.eq_c(k.v(y), k.i32, "1"), k.eq_c(k.v(x), k.i32, wrong ? "0" : "1")));
    f->body = k.blkc({dx, dy, c, k.ret(k.v(y))});
    k.done(f);
    return c->loc;
  };
  lc("seq_cell_assign_value", "int y = (x = 1) on a cell x: the node's value is the stored value and the cell holds it", [=](Kit& k) { asg_cell(k, false); });
  lc("seq_cell_assign_value_wrong", "check(x == 0) after int y = (x = 1)", [=](Kit& k) { k.fail("check", asg_cell(k, true)); });
  // int f() { int x = 0; int y = x++; check(y == 0 && x == 1); return y; }   (the flip of mem_rej_cell_postfix_increment_as_value)
  const auto post_cell = [=](Kit& k, bool wrong) -> SrcLoc {
    Function* f = k.fn("f", k.i32);
    auto [x, dx] = cell_init(k, f, "x", "0");
    Var* y = k.local(f, "y", k.i32);
    const Stmt* dy = k.decl(y, k.incE(x));
    const Stmt* c = k.chk(k.land(k.eq_c(k.v(y), k.i32, "0"), k.eq_c(k.v(x), k.i32, wrong ? "0" : "1")));
    f->body = k.blkc({dx, dy, c, k.ret(k.v(y))});
    k.done(f);
    return c->loc;
  };
  lc("seq_cell_postfix_value", "int y = x++ on a cell: y is the snapshot taken before the write, x holds the new value", [=](Kit& k) { post_cell(k, false); });
  lc("seq_cell_postfix_value_wrong", "check(x == 0) after int y = x++", [=](Kit& k) { k.fail("check", post_cell(k, true)); });
  // x = INT_MAX; int y = x++: the operator's overflow check, before the write effect
  lc("seq_cell_postfix_overflow", "int x = INT_MAX; int y = x++: overflow at the increment", [=](Kit& k) {
    Function* f = k.fn("f", k.vd);
    auto [x, dx] = cell_init(k, f, "x", "2147483647");
    Var* y = k.local(f, "y", k.i32);
    const Expr* inc = k.incE(x);
    f->body = k.blkc({dx, k.decl(y, inc), k.es(k.v(y))});
    k.done(f);
    k.fail("overflow", inc->loc);
  });
  // int x; int y = x++: the read of the cell is the snapshot's
  lc("seq_cell_postfix_uninitialized", "int x; int y = x++: uninitialized at the read of the cell", [=](Kit& k) {
    Function* f = k.fn("f", k.vd);
    Var* x = k.cell(f, "x", k.i32);
    Var* y = k.local(f, "y", k.i32);
    const Expr* inc = k.incE(x);
    f->body = k.blkc({k.decl(x, nullptr), k.decl(y, inc), k.es(k.v(y))});
    k.done(f);
    k.fail("uninitialized", inc->loc);
  });

  // ---- item 2: StoreE, plain, compound and with yields_old ----
  // int x = 1; int *p = &x; a = (*p = 3) + 1; b = (*p += 2); c = (*p)++; d = ++*p; check(a == 4 && b == 5 && c == 5 && d == 7 && x == 7)
  const auto store_values = [=](Kit& k, bool wrong) -> SrcLoc {
    Function* f = k.fn("f", k.vd);
    auto [x, dx] = cell_init(k, f, "x", "1");
    Var* p = k.local(f, "p", PT(k));
    Var* u1 = k.local(f, "a", k.i32);
    Var* u2 = k.local(f, "b", k.i32);
    Var* u3 = k.local(f, "c", k.i32);
    Var* u4 = k.local(f, "d", k.i32);
    const Stmt* dp = k.decl(p, k.addr(x));
    const Expr* s1 = store_e(k, k.pn(p), N(k, "3"));
    const Stmt* d1 = k.decl(u1, k.add(s1, N(k, "1")));
    const Expr* s2 = cstore_e(k, k.pn(p), BinaryOp::Add, N(k, "2"));
    const Stmt* d2 = k.decl(u2, s2);
    const Expr* s3 = idstore_e(k, k.pn(p), true, true);
    const Stmt* d3 = k.decl(u3, s3);
    const Expr* s4 = idstore_e(k, k.pn(p), true, false);
    const Stmt* d4 = k.decl(u4, s4);
    const Stmt* c = k.chk(k.land(k.land(k.eq_c(k.v(u1), k.i32, "4"), k.eq_c(k.v(u2), k.i32, "5")),
                                 k.land(k.land(k.eq_c(k.v(u3), k.i32, "5"), k.eq_c(k.v(u4), k.i32, wrong ? "6" : "7")), k.eq_c(k.v(x), k.i32, "7"))));
    f->body = k.blkc({dx, dp, d1, d2, d3, d4, c});
    k.done(f);
    return c->loc;
  };
  lc("seq_store_value_kinds", "(*p = 3) + 1, (*p += 2), (*p)++ and ++*p used as values: the stored value, the new value, the old value, the new value", [=](Kit& k) { store_values(k, false); });
  lc("seq_store_value_kinds_wrong", "check(d == 6) after ++*p with *p == 6 before", [=](Kit& k) { k.fail("check", store_values(k, true)); });
  // int f(int *p) { int y = (*p = 1); return y; }  with owns(*p) (mode 0), p == NULL || owns(*p) (1), reads(*p) (2)
  const auto store_param = [=](Kit& k, int mode) -> const Expr* {
    Function* f = k.fn("f", k.i32, {{"p", PT(k)}});
    const auto isnull = [&]() { return k.eq(FP(k, f, 0), k.nul(PT(k))); };
    if (mode == 0) {
      k.req(f, owns(k, f, 0, 0));
      k.ens(f, owns(k, f, 0, 0));
    } else if (mode == 1) {
      k.req(f, k.lor(isnull(), owns(k, f, 0, 0)));
      k.ens(f, k.lor(isnull(), owns(k, f, 0, 0)));
    } else {
      k.req(f, reads(k, f, 0, 0));
      k.ens(f, reads(k, f, 0, 0));
    }
    const Var* p = f->param_var(0);
    Var* y = k.local(f, "y", k.i32);
    const Expr* se = store_e(k, k.pn(p), N(k, "1"));
    f->body = k.blk({k.decl(y, se), k.ret(k.v(y))});
    k.done(f);
    return se;
  };
  lc("seq_store_value_owned", "int y = (*p = 1) with owns(*p): the null check and the write permission hold", [=](Kit& k) { store_param(k, 0); });
  lc("seq_store_value_null", "the same with p == NULL || owns(*p): the store has its null check (null-dereference at the store)", [=](Kit& k) {
    k.fail("null-dereference", store_param(k, 1)->loc);
  });
  lc("seq_store_value_reads", "the same with only reads(*p): the write needs the whole permission (permission at the store)", [=](Kit& k) {
    k.fail("permission", store_param(k, 2)->loc);
  });

  // ---- item 5: a compound store whose address has a side effect evaluates it once ----
  // int x = 1; int *p = &x; int *q = NULL; *(q = p) += 1; check(x == 2 && q == p)
  const auto compound_address = [=](Kit& k, bool wrong) -> SrcLoc {
    Function* f = k.fn("f", k.vd);
    auto [x, dx] = cell_init(k, f, "x", "1");
    Var* p = k.local(f, "p", PT(k));
    Var* q = k.local(f, "q", PT(k));
    const Stmt* dp = k.decl(p, k.addr(x));
    const Stmt* dq = k.decl(q, k.nul(PT(k)));
    const Expr* addr = k.asgE(q, k.pn(p));
    const Expr* cs = cstore_e(k, addr, BinaryOp::Add, N(k, "1"));
    const Stmt* c = k.chk(k.land(k.eq_c(k.v(x), k.i32, wrong ? "3" : "2"), k.eq(k.pn(q), k.pn(p))));
    f->body = k.blkc({dx, dp, dq, k.es(cs), c});
    k.done(f);
    return c->loc;
  };
  lc("seq_compound_store_address", "*(q = p) += 1: the address (with its side effect) is evaluated once, x is 2", [=](Kit& k) { compound_address(k, false); });
  lc("seq_compound_store_address_wrong", "check(x == 3) after *(q = p) += 1", [=](Kit& k) { k.fail("check", compound_address(k, true)); });

  // ---- item 3: path-precise obligations ----
  // void f(int c) { int x = 1, y = 5; int *p = &x; int *q = (alias ? (c ? p : &y) : &y); *p = (*q)++; check(x == 5 && y == 6); }
  // The reported access is the outer store, created after the inner write.
  const auto store_postfix = [=](Kit& k, bool alias) -> const Expr* {
    Function* f = k.fn("f", k.vd, {{"c", k.i32}});
    auto [x, dx] = cell_init(k, f, "x", "1");
    auto [y, dy] = cell_init(k, f, "y", "5");
    Var* p = k.local(f, "p", PT(k));
    Var* q = k.local(f, "q", PT(k));
    const Stmt* dp = k.decl(p, k.addr(x));
    const Stmt* dq = k.decl(q, alias ? k.cond(k.ne(k.pv(f, 0), N(k, "0")), k.pn(p), k.addr(y)) : k.addr(y));
    const Expr* inner = idstore_e(k, k.pn(q), true, true);
    const Expr* outer = store_e(k, k.pn(p), inner);
    std::vector<const Stmt*> body = {dx, dy, dp, dq, k.es(outer)};
    if (!alias) body.push_back(k.chk(k.land(k.eq_c(k.v(x), k.i32, "5"), k.eq_c(k.v(y), k.i32, "6"))));
    f->body = k.blkc(body);
    k.done(f);
    return outer;
  };
  lc("seq_store_postfix_distinct", "*p = (*q)++ with p = &x and q = &y: p != q holds, x == 5 and y == 6", [=](Kit& k) { store_postfix(k, false); });
  lc("seq_store_postfix_alias", "q = c ? p : &y: on the path c != 0 *p = (*q)++ names one object twice: unsequenced at the later access",
     [=](Kit& k) { k.fail("unsequenced", store_postfix(k, true)->loc); });
  // void f(int c) { int x = 1, y = 10; int *p = (alias ? (c ? &x : &y) : &y); int s = x++ + *p; check(s == 11 && x == 2); }
  const auto incr_plus_read = [=](Kit& k, bool alias) -> const Expr* {
    Function* f = k.fn("f", k.vd, {{"c", k.i32}});
    auto [x, dx] = cell_init(k, f, "x", "1");
    auto [y, dy] = cell_init(k, f, "y", "10");
    Var* p = k.local(f, "p", PT(k));
    Var* s = k.local(f, "s", k.i32);
    const Stmt* dp = k.decl(p, alias ? k.cond(k.ne(k.pv(f, 0), N(k, "0")), k.addr(x), k.addr(y)) : k.addr(y));
    const Expr* a = k.incE(x);
    const Expr* d = k.deref(k.pn(p));
    const Stmt* ds = k.decl(s, k.add(a, d));
    std::vector<const Stmt*> body = {dx, dy, dp, ds};
    if (!alias) body.push_back(k.chk(k.land(k.eq_c(k.v(s), k.i32, "11"), k.eq_c(k.v(x), k.i32, "2"))));
    f->body = k.blkc(body);
    k.done(f);
    return d;
  };
  lc("seq_incr_plus_read_distinct", "int s = x++ + *p with p = &y: x != p holds, s == 11", [=](Kit& k) { incr_plus_read(k, false); });
  lc("seq_incr_plus_read_alias", "p = c ? &x : &y: x++ + *p modifies and reads one object on the path c != 0: unsequenced at the read",
     [=](Kit& k) { k.fail("unsequenced", incr_plus_read(k, true)->loc); });
  // void f(int c) { x = 1, y = 2, z = 3, w = 4; a = &x; b = &y; q = &z; s = (alias ? (c ? q : &w) : &w); *(*q > 0 ? a : b) = (*s)++; }
  // The target's address reads *q, the value writes *s: audit S1.
  const auto cond_target = [=](Kit& k, bool alias) -> const Expr* {
    Function* f = k.fn("f", k.vd, {{"c", k.i32}});
    auto [x, dx] = cell_init(k, f, "x", "1");
    auto [y, dy] = cell_init(k, f, "y", "2");
    auto [z, dz] = cell_init(k, f, "z", "3");
    auto [w, dw] = cell_init(k, f, "w", "4");
    Var* a = k.local(f, "a", PT(k));
    Var* b = k.local(f, "b", PT(k));
    Var* q = k.local(f, "q", PT(k));
    Var* s = k.local(f, "s", PT(k));
    const Stmt* da = k.decl(a, k.addr(x));
    const Stmt* db = k.decl(b, k.addr(y));
    const Stmt* dq = k.decl(q, k.addr(z));
    const Stmt* ds = k.decl(s, alias ? k.cond(k.ne(k.pv(f, 0), N(k, "0")), k.pn(q), k.addr(w)) : k.addr(w));
    const Expr* rdq = k.deref(k.pn(q));
    const Expr* target = k.cond(k.gt(rdq, N(k, "0")), k.pn(a), k.pn(b));
    const Expr* inner = idstore_e(k, k.pn(s), true, true);
    const Stmt* st = k.store(target, inner);
    std::vector<const Stmt*> body = {dx, dy, dz, dw, da, db, dq, ds, st};
    if (!alias) body.push_back(k.chk(k.land(k.land(k.eq_c(k.v(x), k.i32, "4"), k.eq_c(k.v(y), k.i32, "2")), k.eq_c(k.v(w), k.i32, "5"))));
    f->body = k.blkc(body);
    k.done(f);
    return inner;
  };
  lc("seq_cond_target_distinct", "*(*q > 0 ? a : b) = (*s)++ with s = &w: the address's read of *q and the value's write of *s are different cells", [=](Kit& k) { cond_target(k, false); });
  lc("seq_cond_target_alias", "s = c ? q : &w: on the path c != 0 the address reads *q and the value writes it: unsequenced at the later access",
     [=](Kit& k) { k.fail("unsequenced", cond_target(k, true)->loc); });

  // ---- item 4: a call and an access C does not order ----
  // int f() { int x = 1; int y = 2; int s = x + bump(&y); x = bump(&x); return s + x; }   (ens r == 6)
  lc("seq_call_beside_other_cell", "x + bump(&y), then x = bump(&x) as a statement: no obligation fails, the store follows the call", [=](Kit& k) {
    Function* bump = make_bump(k);
    Function* f = k.fn("f", k.i32);
    k.ens(f, k.eq_c(k.R(f), k.i32, "6"));
    auto [x, dx] = cell_init(k, f, "x", "1");
    auto [y, dy] = cell_init(k, f, "y", "2");
    Var* s = k.local(f, "s", k.i32);
    const Expr* rd = k.v(x);
    const Expr* c1 = k.call(bump, {k.addr(y)});
    const Stmt* ds = k.decl(s, k.add(rd, c1));
    const Stmt* st = k.asg(x, k.call(bump, {k.addr(x)}));
    f->body = k.blkc({dx, dy, ds, st, k.ret(k.add(k.v(s), k.v(x)))});
    k.done(f);
  });
  // int f() { int x = 1; return x + bump(&x); }   or   x = x + bump(&x)   (the read of the target)
  const auto read_plus_call = [=](Kit& k, bool compound) -> const Expr* {
    Function* bump = make_bump(k);
    Function* f = k.fn("f", k.i32);
    auto [x, dx] = cell_init(k, f, "x", "1");
    const Expr* rd = k.v(x);
    const Expr* c1 = k.call(bump, {k.addr(x)});
    const Expr* sum = k.add(rd, c1);
    if (compound) f->body = k.blkc({dx, k.asg(x, sum), k.ret(k.v(x))});
    else f->body = k.blkc({dx, k.ret(sum)});
    k.done(f);
    return rd;
  };
  lc("seq_call_beside_read", "return x + bump(&x): the read of x and the call may be evaluated in either order: unsequenced at the read",
     [=](Kit& k) { k.fail("unsequenced", read_plus_call(k, false)->loc); });
  lc("seq_call_beside_compound_target", "x += bump(&x): the read of the target is an access C does not order with the call: unsequenced at the read",
     [=](Kit& k) { k.fail("unsequenced", read_plus_call(k, true)->loc); });
  // int f() { int x = 1; int y = 2; return mode: 0 x + peek(&x) (reads atom, a read: no obligation), 1 (x = 2) + peek(&y), 2 (x = 2) + peek(&x) }
  const auto beside_reads = [=](Kit& k, int mode) -> const Expr* {
    Function* peek = make_peek(k);
    Function* f = k.fn("f", k.i32);
    if (mode == 0) k.ens(f, k.eq_c(k.R(f), k.i32, "2"));
    if (mode == 1) k.ens(f, k.eq_c(k.R(f), k.i32, "4"));
    auto [x, dx] = cell_init(k, f, "x", "1");
    auto [y, dy] = cell_init(k, f, "y", "2");
    const Expr* first = mode == 0 ? k.v(x) : k.asgE(x, N(k, "2"));
    const Expr* c1 = k.call(peek, {mode == 1 ? k.addr(y) : k.addr(x)});
    f->body = k.blkc({dx, dy, k.ret(k.add(first, c1))});
    k.done(f);
    return first;
  };
  lc("seq_call_reads_beside_read", "x + peek(&x): a reads atom and a read of the cell may share a full expression", [=](Kit& k) { beside_reads(k, 0); });
  lc("seq_call_reads_beside_other_write", "(x = 2) + peek(&y): the write and the reads atom name different cells", [=](Kit& k) { beside_reads(k, 1); });
  lc("seq_call_reads_beside_write", "(x = 2) + peek(&x): a write against any atom of the call: unsequenced at the write",
     [=](Kit& k) { k.fail("unsequenced", beside_reads(k, 2)->loc); });
  // int f(c) { int x = 1, y = 2; mode 0: bump(&x) + bump(&y), 1: bump(&x) + peek(&y), 2: bump(&x) + bump(&x), 3: bump(p) + peek(q) with q = c ? p : &y }
  const auto two_calls = [=](Kit& k, int mode) -> const Expr* {
    Function* bump = make_bump(k);
    Function* peek = make_peek(k);
    Function* f = k.fn("f", k.i32, {{"c", k.i32}});
    if (mode == 0) k.ens(f, k.eq_c(k.R(f), k.i32, "5"));
    if (mode == 1) k.ens(f, k.eq_c(k.R(f), k.i32, "4"));
    auto [x, dx] = cell_init(k, f, "x", "1");
    auto [y, dy] = cell_init(k, f, "y", "2");
    std::vector<const Stmt*> body = {dx, dy};
    const Expr* c1 = nullptr;
    const Expr* c2 = nullptr;
    if (mode == 3) {
      Var* p = k.local(f, "p", PT(k));
      Var* q = k.local(f, "q", PT(k));
      body.push_back(k.decl(p, k.addr(x)));
      body.push_back(k.decl(q, k.cond(k.ne(k.pv(f, 0), N(k, "0")), k.pn(p), k.addr(y))));
      c1 = k.call(bump, {k.pn(p)});
      c2 = k.call(peek, {k.pn(q)});
    } else {
      c1 = k.call(bump, {k.addr(x)});
      c2 = mode == 0 ? k.call(bump, {k.addr(y)}) : mode == 1 ? k.call(peek, {k.addr(y)}) : k.call(bump, {k.addr(x)});
    }
    body.push_back(k.ret(k.add(c1, c2)));
    f->body = k.blkc(body);
    k.done(f);
    return c2;
  };
  lc("seq_two_calls_distinct", "bump(&x) + bump(&y): arg1 != arg2 holds", [=](Kit& k) { two_calls(k, 0); });
  lc("seq_two_calls_owns_reads", "bump(&x) + peek(&y): an owns atom and a reads atom on different cells", [=](Kit& k) { two_calls(k, 1); });
  lc("seq_two_calls_same", "bump(&x) + bump(&x): both own *x and C does not order the calls: unsequenced at the later call",
     [=](Kit& k) { k.fail("unsequenced", two_calls(k, 2)->loc); });
  lc("seq_two_calls_alias", "bump(p) + peek(q) with q = c ? p : &y: the owns and the reads atom may name one cell: unsequenced at the later call",
     [=](Kit& k) { k.fail("unsequenced", two_calls(k, 3)->loc); });

  // ---- item 4: no reordering across a memory call; B1's snapshot after its guard ----
  // int f() { int x = 1; int y = (x = (bump(&x), 2)); check(y == 2 && x == 2); return y; }
  const auto write_after_call = [=](Kit& k, bool wrong) -> SrcLoc {
    Function* bump = make_bump(k);
    Function* f = k.fn("f", k.i32);
    auto [x, dx] = cell_init(k, f, "x", "1");
    Var* y = k.local(f, "y", k.i32);
    const Expr* call = k.call(bump, {k.addr(x)});
    const Expr* cm = k.comma(call, N(k, "2"));
    const Expr* ae = k.asgE(x, cm);
    const Stmt* dy = k.decl(y, ae);
    const Stmt* c = k.chk(k.land(k.eq_c(k.v(y), k.i32, "2"), k.eq_c(k.v(x), k.i32, wrong ? "3" : "2")));
    f->body = k.blkc({dx, dy, c, k.ret(k.v(y))});
    k.done(f);
    return c->loc;
  };
  lc("seq_write_after_call", "y = (x = (bump(&x), 2)): the write follows the call (x == 2, not the 3 the write-first order would give)", [=](Kit& k) { write_after_call(k, false); });
  lc("seq_write_after_call_wrong", "check(x == 3): the value the wrong order would give", [=](Kit& k) { k.fail("check", write_after_call(k, true)); });
  // bool f() ensures(!r) { int x = 0; return bump(&x) && x == 0; }   (B1: the read in the right operand follows the call)
  const auto and_after_call = [=](Kit& k, bool wrong) -> SrcLoc {
    Function* bump = make_bump(k);
    Function* f = k.fn("f", k.bl);
    k.ens(f, wrong ? k.R(f) : k.lnot(k.R(f)));
    auto [x, dx] = cell_init(k, f, "x", "0");
    const Expr* call = k.call(bump, {k.addr(x)});
    const Expr* rd = k.v(x);
    const Stmt* r = k.ret(k.land(k.tb(call), k.eq_c(rd, k.i32, "0")));
    f->body = k.blkc({dx, r});
    k.done(f);
    return r->loc;
  };
  lc("seq_and_after_call", "return bump(&x) && x == 0: bump makes x 1, so the read in the right operand gives false", [=](Kit& k) { and_after_call(k, false); });
  lc("seq_and_after_call_wrong", "ensures(r): the value a read before the call would give", [=](Kit& k) { k.fail("postcondition", and_after_call(k, true)); });

  // ---- B2: a call that never returns does not hide a check C may evaluate first ----
  // int f(int *r, int *q) { return (*r = nr()) + (*q = 1); }  with owns(*q) (mode 0) or q == NULL || owns(*q) (1)
  const auto nr_store = [=](Kit& k, int mode) -> const Expr* {
    Function* nr = make_nr(k);
    Function* f = k.fn("f", k.i32, {{"r", PT(k)}, {"q", PT(k)}});
    const auto isnull = [&]() { return k.eq(FP(k, f, 1), k.nul(PT(k))); };
    if (mode == 0) {
      k.req(f, k.land(owns(k, f, 0, 0), owns(k, f, 1, 1)));
      k.ens(f, k.land(owns(k, f, 0, 0), owns(k, f, 1, 1)));
    } else {
      k.req(f, k.land(owns(k, f, 0, 0), k.lor(isnull(), owns(k, f, 1, 1))));
      k.ens(f, k.land(owns(k, f, 0, 0), k.lor(isnull(), owns(k, f, 1, 1))));
    }
    const Var* r = f->param_var(0);
    const Var* q = f->param_var(1);
    const Expr* s1 = store_e(k, k.pn(r), k.call(nr, {}));
    const Expr* s2 = store_e(k, k.pn(q), N(k, "1"));
    f->body = k.blk({k.ret(k.add(s1, s2))});
    k.done(f);
    return s2;
  };
  lc("seq_noreturn_store_owned", "(*r = nr()) + (*q = 1) with both owned: nothing fails (nr never returns)", [=](Kit& k) { nr_store(k, 0); });
  lc("seq_noreturn_store_null", "q may be null: C may evaluate *q = 1 before nr(), so its null check stays before the call (null-dereference at the *q store)",
     [=](Kit& k) { k.fail("null-dereference", nr_store(k, 1)->loc); });
  // int f(int *q, int *r) { return (*q = nr()) + 100 / *r; }  with *r != 0 required (ok) or not
  const auto nr_divide = [=](Kit& k, bool nonzero) -> const Expr* {
    Function* nr = make_nr(k);
    Function* f = k.fn("f", k.i32, {{"q", PT(k)}, {"r", PT(k)}});
    k.req(f, k.land(k.land(owns(k, f, 0, 0), owns(k, f, 1, 1)), nonzero ? k.ne(star(k, f, 1), N(k, "0")) : k.yes()));
    k.ens(f, k.land(owns(k, f, 0, 0), owns(k, f, 1, 1)));
    const Var* q = f->param_var(0);
    const Var* r = f->param_var(1);
    const Expr* s1 = store_e(k, k.pn(q), k.call(nr, {}));
    const Expr* dv = k.div(N(k, "100"), k.deref(k.pn(r)));
    f->body = k.blk({k.ret(k.add(s1, dv))});
    k.done(f);
    return dv;
  };
  lc("seq_noreturn_divide_nonzero", "(*q = nr()) + 100 / *r with *r != 0: nothing fails", [=](Kit& k) { nr_divide(k, true); });
  lc("seq_noreturn_divide_zero", "*r may be 0: C may evaluate 100 / *r before nr(): division-by-zero at the operator",
     [=](Kit& k) { k.fail("division-by-zero", nr_divide(k, false)->loc); });

  // ---- S2: an access whose place is a call's value follows the call ----
  // int f() { int x = 1; int v = *first(&x); check(v == 1 && x == 1); return v; }
  const auto result_place = [=](Kit& k, bool wrong) -> SrcLoc {
    Function* first = make_first(k);
    Function* f = k.fn("f", k.i32);
    auto [x, dx] = cell_init(k, f, "x", "1");
    Var* v = k.local(f, "v", k.i32);
    const Stmt* dv = k.decl(v, k.deref(k.call(first, {k.addr(x)})));
    const Stmt* c = k.chk(k.land(k.eq_c(k.v(v), k.i32, wrong ? "2" : "1"), k.eq_c(k.v(x), k.i32, "1")));
    f->body = k.blkc({dx, dv, c, k.ret(k.v(v))});
    k.done(f);
    return c->loc;
  };
  lc("seq_call_result_place", "int v = *first(&x): the place is the call's value, the read follows the call (no unsequenced report)", [=](Kit& k) { result_place(k, false); });
  lc("seq_call_result_place_wrong", "check(v == 2) after int v = *first(&x)", [=](Kit& k) { k.fail("check", result_place(k, true)); });

  // ---- item 7: conditional ownership, the overlap assert, partners by path (question 72) ----
  // void step(int c, int *p) requires(c ? owns(*p) : reads(*p)) ensures((c ? owns(*p) : reads(*p)) && *p == old(*p))  (trusted, or an empty body)
  const auto make_lend = [=](Kit& k, const char* name, bool trusted) -> Function* {
    Function* f = k.fn(name, k.vd, {{"c", k.i32}, {"p", PT(k)}});
    k.req(f, k.cond(k.ne(k.F(f, 0), N(k, "0")), owns(k, f, 1, 0), reads(k, f, 1, 1)));
    k.ens(f, k.land(k.cond(k.ne(k.F(f, 0), N(k, "0")), owns(k, f, 1, 0), reads(k, f, 1, 1)), k.eq(star(k, f, 1), old_star(k, f, 1))));
    if (trusted) {
      k.trusted(f);
    } else {
      f->body = k.blk({});
    }
    k.done(f);
    return f;
  };
  lc("seq_conditional_trusted_callers", "bodyless and empty-bodied c ? owns(*p) : reads(*p) contracts, called with 1, 0 and an unknown c: the caller keeps its cell and its value", [=](Kit& k) {
    Function* lend = make_lend(k, "lend", true);
    Function* lend_body = make_lend(k, "lend_body", false);
    Function* f = k.fn("f", k.i32, {{"c", k.i32}});
    k.ens(f, k.eq_c(k.R(f), k.i32, "7"));
    auto [x, dx] = cell_init(k, f, "x", "7");
    std::vector<const Stmt*> body = {dx};
    for (Function* callee : {lend, lend_body}) {
      body.push_back(k.es(k.call(callee, {N(k, "1"), k.addr(x)})));
      body.push_back(k.es(k.call(callee, {N(k, "0"), k.addr(x)})));
      body.push_back(k.es(k.call(callee, {k.pv(f, 0), k.addr(x)})));
    }
    body.push_back(k.ret(k.v(x)));
    f->body = k.blkc(body);
    k.done(f);
  });
  // void both(int c, int *p) requires((c != 0 || owns(*p)) && reads(*p)) {}
  lc("seq_conditional_overlap", "(c != 0 || owns(*p)) && reads(*p): both atoms hold when c == 0: the contract owns *p and reads it on one path (permission at the later atom)", [=](Kit& k) {
    Function* f = k.fn("both", k.vd, {{"c", k.i32}, {"p", PT(k)}});
    const Expr* ow = owns(k, f, 1, 0);
    const Expr* rd = reads(k, f, 1, 1);
    k.req(f, k.land(k.lor(k.ne(k.F(f, 0), N(k, "0")), ow), rd));
    f->body = k.blk({});
    k.done(f);
    k.fail("permission", rd->loc);
  });
  // two clauses of the same contract
  lc("seq_conditional_overlap_clauses", "requires(owns(*p)) and requires(c == 0 || reads(*p)) in two clauses: both hold when c == 0 (permission at the later atom)", [=](Kit& k) {
    Function* f = k.fn("both", k.vd, {{"c", k.i32}, {"p", PT(k)}});
    const Expr* ow = owns(k, f, 1, 0);
    const Expr* rd = reads(k, f, 1, 1);
    k.req(f, ow);
    k.req(f, k.lor(k.eq_c(k.F(f, 0), k.i32, "0"), rd));
    f->body = k.blk({});
    k.done(f);
    k.fail("permission", rd->loc);
  });
  // void lend(int c, int *p) requires(c ? owns(*p) : reads(*p)) ensures(c == 0 || reads(*p)) ensures(c != 0 || owns(*p));   (trusted; the swap)
  lc("seq_conditional_partners_swapped", "a bodyless contract that returns reads(*p) when c and owns(*p) otherwise: each ensures atom holds where its partner was not taken (permission at each ensures atom)", [=](Kit& k) {
    Function* f = k.fn("lend", k.vd, {{"c", k.i32}, {"p", PT(k)}});
    k.req(f, k.cond(k.ne(k.F(f, 0), N(k, "0")), owns(k, f, 1, 0), reads(k, f, 1, 1)));
    const Expr* rd = reads(k, f, 1, 1);
    const Expr* ow = owns(k, f, 1, 0);
    k.ens(f, k.lor(k.eq_c(k.F(f, 0), k.i32, "0"), rd));
    k.ens(f, k.lor(k.ne(k.F(f, 0), N(k, "0")), ow));
    k.trusted(f);
    k.done(f);
    Function* h = k.fn("h", k.vd, {{"p", PT(k)}});
    k.req(h, owns(k, h, 0, 0));
    k.ens(h, owns(k, h, 0, 0));
    h->body = k.blk({k.es(k.call(f, {N(k, "0"), k.pn(h->param_var(0))}))});
    k.done(h);
    k.fail("permission", rd->loc);
    // the later atom carries item 7's overlap assert (it passes: the guards are exclusive) and question 72's partner assert (it fails)
    k.fail_only("permission", ow->loc, 2, 1);
  });
  // void upgrade(int c, int *p) requires(c ? owns(*p) : live(p)) ensures(owns(*p));   (trusted)
  lc("seq_conditional_partners_live", "requires(c ? owns(*p) : live(p)) with ensures(owns(*p)): the ensures atom holds on the path c == 0 where only liveness was taken (permission at the ensures atom)", [=](Kit& k) {
    Function* f = k.fn("upgrade", k.vd, {{"c", k.i32}, {"p", PT(k)}});
    k.req(f, k.cond(k.ne(k.F(f, 0), N(k, "0")), owns(k, f, 1, 0), live(k, f, 1, 1)));
    const Expr* ow = owns(k, f, 1, 0);
    k.ens(f, ow);
    k.trusted(f);
    k.done(f);
    k.fail("permission", ow->loc);
  });

  // ---- item 8: a function's measure over memory ----
  // int down(int *p) requires(owns(*p) && *p >= 0 && *p < 100) decreases(*p) ensures(owns(*p)) { if (*p == 0) return 0; [*p -= 1;] return down(p); }
  const auto make_down = [=](Kit& k, bool decrement) -> const Expr* {
    Function* f = k.fn("down", k.i32, {{"p", PT(k)}});
    k.req(f, k.land(owns(k, f, 0, 0), k.land(k.ge_c(star(k, f, 0), k.i32, "0"), k.lt(star(k, f, 0), N(k, "100")))));
    k.dec(f, star(k, f, 0));
    k.ens(f, owns(k, f, 0, 0));
    const Var* p = f->param_var(0);
    const Expr* rec = k.call(f, {k.pn(p)});
    std::vector<const Stmt*> body = {k.iff(k.eq(k.deref(k.pn(p)), N(k, "0")), k.blk({k.ret(N(k, "0"))}))};
    if (decrement) body.push_back(k.store(k.pn(p), k.sub(k.deref(k.pn(p)), N(k, "1"))));
    body.push_back(k.ret(rec));
    f->body = k.blk(body);
    k.done(f);
    return rec;
  };
  lc("seq_measure_over_memory", "decreases(*p) over an owned cell: the recursive call reads the callee's measure in its entry state", [=](Kit& k) { make_down(k, true); });
  lc("seq_measure_over_memory_same", "the same without the decrement: *p is the same at the call (termination at the recursive call)",
     [=](Kit& k) { k.fail("termination", make_down(k, false)->loc); });
  lc("seq_measure_leaf", "a function that is not recursive may have decreases(*p) too (the framing is checked for every function)", [=](Kit& k) {
    Function* f = k.fn("leaf", k.i32, {{"p", PT(k)}});
    k.req(f, k.land(owns(k, f, 0, 0), k.ge_c(star(k, f, 0), k.i32, "0")));
    k.dec(f, star(k, f, 0));
    k.ens(f, k.land(owns(k, f, 0, 0), k.eq(k.R(f), star(k, f, 0))));
    f->body = k.blk({k.ret(k.deref(k.pn(f->param_var(0))))});
    k.done(f);
  });
  lc("seq_measure_unframed", "decreases(*p) with only live(p): the measure reads *p without owning it (permission at the measure's read)", [=](Kit& k) {
    Function* f = k.fn("unframed", k.i32, {{"p", PT(k)}});
    k.req(f, live(k, f, 0, 0));
    const Expr* d = star(k, f, 0);
    k.dec(f, d);
    f->body = k.blk({k.ret(N(k, "0"))});
    k.done(f);
    k.fail("permission", d->loc);
  });
  // int down(int *p, int *q) requires(owns(*p) && reads(*q) && 0 <= *p < 100) decreases(*p) ensures(owns(*p) && reads(*q) && r == 0)
  //   { if (*p == 0) return 0; [*p -= 1;] return peek(q) * 0 + down(p, q); }   (0021 item 4's explicit check is printed for the recursive call)
  const auto make_down_after_call = [=](Kit& k, bool decrement) -> const Expr* {
    Function* peek = make_peek(k);
    Function* f = k.fn("down", k.i32, {{"p", PT(k)}, {"q", PT(k)}});
    k.req(f, k.land(k.land(owns(k, f, 0, 0), reads(k, f, 1, 1)), k.land(k.ge_c(star(k, f, 0), k.i32, "0"), k.lt(star(k, f, 0), N(k, "100")))));
    k.dec(f, star(k, f, 0));
    k.ens(f, k.land(k.land(owns(k, f, 0, 0), reads(k, f, 1, 1)), k.eq_c(k.R(f), k.i32, "0")));
    const Var* p = f->param_var(0);
    const Var* q = f->param_var(1);
    const Expr* pk = k.call(peek, {k.pn(q)});
    const Expr* m = k.mul(pk, N(k, "0"));
    const Expr* rec = k.call(f, {k.pn(p), k.pn(q)});
    std::vector<const Stmt*> body = {k.iff(k.eq(k.deref(k.pn(p)), N(k, "0")), k.blk({k.ret(N(k, "0"))}))};
    if (decrement) body.push_back(k.store(k.pn(p), k.sub(k.deref(k.pn(p)), N(k, "1"))));
    body.push_back(k.ret(k.add(m, rec)));
    f->body = k.blk(body);
    k.done(f);
    return rec;
  };
  lc("seq_measure_after_call", "the recursive call follows peek(q) in its expression: the explicit decrease check reads the measure over the current heap and the caller's under old", [=](Kit& k) { make_down_after_call(k, true); });
  lc("seq_measure_after_call_same", "the same without the decrement: the explicit check of the call after peek fails (termination at the call)",
     [=](Kit& k) {
       // the explicit assert (0021 item 4) fails and Silicon's own check at the call is the backstop: two rows at the call, one report
       k.fail_only("termination", make_down_after_call(k, false)->loc, 2, 0);
     });
  // B2: the recursive call beside nr(): the explicit check must not be hidden
  // int down(int *p) ... { int a = 0, b = 0; int *q = &a, *s = &b; if (*p == 0) return 0; [*p -= 1;] return (*q = nr()) + (*s = 1, down(p)); }
  const auto make_down_nr = [=](Kit& k, bool decrement) -> const Expr* {
    Function* nr = make_nr(k);
    Function* f = k.fn("down", k.i32, {{"p", PT(k)}});
    k.req(f, k.land(owns(k, f, 0, 0), k.land(k.ge_c(star(k, f, 0), k.i32, "0"), k.lt(star(k, f, 0), N(k, "100")))));
    k.dec(f, star(k, f, 0));
    k.ens(f, owns(k, f, 0, 0));
    const Var* p = f->param_var(0);
    auto [a, da] = cell_init(k, f, "a", "0");
    auto [b, db] = cell_init(k, f, "b", "0");
    Var* q = k.local(f, "q", PT(k));
    Var* s = k.local(f, "s", PT(k));
    const Stmt* dq = k.decl(q, k.addr(a));
    const Stmt* ds = k.decl(s, k.addr(b));
    const Expr* s1 = store_e(k, k.pn(q), k.call(nr, {}));
    const Expr* s2 = store_e(k, k.pn(s), N(k, "1"));
    const Expr* rec = k.call(f, {k.pn(p)});
    const Expr* tail = k.comma(s2, rec);
    std::vector<const Stmt*> body = {da, db, dq, ds, k.iff(k.eq(k.deref(k.pn(p)), N(k, "0")), k.blk({k.ret(N(k, "0"))}))};
    if (decrement) body.push_back(k.store(k.pn(p), k.sub(k.deref(k.pn(p)), N(k, "1"))));
    body.push_back(k.ret(k.add(s1, tail)));
    f->body = k.blkc(body);
    k.done(f);
    return rec;
  };
  lc("seq_noreturn_recursive_call", "(*q = nr()) + (*s = 1, down(p)) with a decrement: the call decreases, nr() never returns", [=](Kit& k) { make_down_nr(k, true); });
  lc("seq_noreturn_recursive_call_same", "without the decrement the call does not decrease, and nr() must not hide it (termination at the call)",
     [=](Kit& k) {
       // the explicit assert (0021 item 4) fails and Silicon's own check at the call is the backstop: two rows at the call, one report
       k.fail_only("termination", make_down_nr(k, false)->loc, 2, 0);
     });

  // ---- audit round 2, R2-B1: a cell write in the left operand of && is complete before the guarded read ----
  // int *pick(int *a) requires(live(a)) ensures(live(a) && r == a);   (trusted)
  // int f(int *q) requires(owns(*q) && *q == 0) ensures(owns(*q) && r == value) { return ((*pick(q) = 5) == 5) && *q == 5; }   (C: the result is 1)
  const auto store_then_read = [=](Kit& k, bool wrong) -> SrcLoc {
    Function* pick = k.fn("pick", PT(k), {{"a", PT(k)}});
    k.req(pick, live(k, pick, 0, 0));
    k.ens(pick, k.land(live(k, pick, 0, 0), k.eq(k.R(pick), FP(k, pick, 0))));
    k.trusted(pick);
    Function* f = k.fn("f", k.i32, {{"q", PT(k)}});
    k.req(f, k.land(owns(k, f, 0, 0), k.eq_c(star(k, f, 0), k.i32, "0")));
    k.ens(f, k.land(owns(k, f, 0, 0), k.eq_c(k.R(f), k.i32, wrong ? "0" : "1")));
    const Var* q = f->param_var(0);
    const Expr* call = k.call(pick, {k.pn(q)});
    const Expr* se = store_e(k, call, N(k, "5"));
    const Expr* left = k.eq(se, N(k, "5"));
    const Expr* right = k.eq(k.deref(k.pn(q)), N(k, "5"));
    const Stmt* r = k.ret(k.bi(k.i32, k.land(left, right)));
    f->body = k.blk({r});
    k.done(f);
    return r->loc;
  };
  lc("seq_store_then_read_and", "return ((*pick(q) = 5) == 5) && *q == 5: the store is complete before the right operand reads *q, the result is 1", [=](Kit& k) { store_then_read(k, false); });
  lc("seq_store_then_read_and_wrong", "ensures r == 0: the value a read of *q before the store would give (postcondition at the return)",
     [=](Kit& k) { k.fail("postcondition", store_then_read(k, true)); });

  // ---- the fail-closed hooks of item 4 ----
  // The program of seq_write_after_call: the hook moves the cell write before the call.
  rh("seq_hook_perturb_memory_order", "wrong side of a memory call", [=](Kit& k) {
    k.lower_options.perturb_memory_order = true;
    write_after_call(k, false);
  });
  // The program of seq_and_after_call: the hook makes the snapshot of x not follow its guard (the call).
  rh("seq_hook_drop_snapshot_guard_set", "wrong side of a memory call", [=](Kit& k) {
    k.lower_options.drop_snapshot_guard_set = true;
    and_after_call(k, false);
  });
  // The program of seq_store_then_read_and (R2-B1): the hook drops the left operand's floor, so the guarded read is emitted before the store.
  rh("seq_hook_drop_left_floor", "wrong side of another access", [=](Kit& k) {
    k.lower_options.drop_left_floor = true;
    store_then_read(k, false);
  });
}

void define_sequencing_rejections() {
  auto PT = [](Kit& k) { return k.ptr(k.i32); };
  auto N = [](Kit& k, const char* v) { return k.num(k.i32, v); };
  auto idstore_e = [](Kit& k, const Expr* addr, bool increment, bool post) { return k.b.inc_dec_store_expr(addr, increment, post, k.at()); };

  // ---- the static half (item 3): objects that compilers warn about ----
  // int f() { int x = 1; return x++ + x; }   (a cell by name is its variable)
  rc("seq_rej_cell_postfix_plus_name", "unsequenced modification of 'x'", [=](Kit& k) {
    Function* f = k.fn("f", k.i32);
    Var* x = k.cell(f, "x", k.i32);
    f->body = k.blkc({k.decl(x, N(k, "1")), k.ret(k.add(k.incE(x), k.v(x)))});
    k.done(f);
  });
  // int f() { int x = 1; int *p = &x; return (*p)++ + (*p)++; }   (`*v` for a pointer variable the expression does not write)
  rc("seq_rej_pointer_postfix_twice", "unsequenced modification of '*p'", [=](Kit& k) {
    Function* f = k.fn("f", k.i32);
    Var* x = k.cell(f, "x", k.i32);
    Var* p = k.local(f, "p", PT(k));
    const Expr* a = idstore_e(k, k.pn(p), true, true);
    const Expr* b = idstore_e(k, k.pn(p), true, true);
    f->body = k.blkc({k.decl(x, N(k, "1")), k.decl(p, k.addr(x)), k.ret(k.add(a, b))});
    k.done(f);
  });
  // int f() { int x = 1; int *p = &x; *p = (*p)++; return x; }   (P61 A4: GCC warns)
  rc("seq_rej_pointer_store_own_postfix", "unsequenced modification of '*p'", [=](Kit& k) {
    Function* f = k.fn("f", k.i32);
    Var* x = k.cell(f, "x", k.i32);
    Var* p = k.local(f, "p", PT(k));
    const Expr* inner = idstore_e(k, k.pn(p), true, true);
    f->body = k.blkc({k.decl(x, N(k, "1")), k.decl(p, k.addr(x)), k.store(k.pn(p), inner), k.ret(k.v(x))});
    k.done(f);
  });
  // int f() { int x = 1; *&x = x++; return x; }   (audit S9: `*&x` is x)
  rc("seq_rej_deref_address_postfix", "unsequenced modification of 'x'", [=](Kit& k) {
    Function* f = k.fn("f", k.i32);
    Var* x = k.cell(f, "x", k.i32);
    const Expr* inc = k.incE(x);
    f->body = k.blkc({k.decl(x, N(k, "1")), k.store(k.addr(x), inc), k.ret(k.v(x))});
    k.done(f);
  });
  // int f() { int x = 1, y = 2; int *p = &x; int *q = &y; return (p = q, (*p)++ + *p); }   (audit S9: GCC warns)
  rc("seq_rej_comma_pointer_postfix", "unsequenced modification of '*p'", [=](Kit& k) {
    Function* f = k.fn("f", k.i32);
    Var* x = k.cell(f, "x", k.i32);
    Var* y = k.cell(f, "y", k.i32);
    Var* p = k.local(f, "p", PT(k));
    Var* q = k.local(f, "q", PT(k));
    const Expr* set = k.asgE(p, k.pn(q));
    const Expr* inc = idstore_e(k, k.pn(p), true, true);
    const Expr* rd = k.deref(k.pn(p));
    const Expr* sum = k.add(inc, rd);
    f->body = k.blkc({k.decl(x, N(k, "1")), k.decl(y, N(k, "2")), k.decl(p, k.addr(x)), k.decl(q, k.addr(y)), k.ret(k.comma(set, sum))});
    k.done(f);
  });
  // int f() { int x = 1, y = 2; int *p = &x; int *q = &y; *(*q > 0 ? p : q) = (*q)++; return x; }   (S1, static: one pointer variable)
  rc("seq_rej_conditional_target_same_pointer", "unsequenced modification of '*q'", [=](Kit& k) {
    Function* f = k.fn("f", k.i32);
    Var* x = k.cell(f, "x", k.i32);
    Var* y = k.cell(f, "y", k.i32);
    Var* p = k.local(f, "p", PT(k));
    Var* q = k.local(f, "q", PT(k));
    const Expr* rdq = k.deref(k.pn(q));
    const Expr* target = k.cond(k.gt(rdq, N(k, "0")), k.pn(p), k.pn(q));
    const Expr* inner = idstore_e(k, k.pn(q), true, true);
    f->body = k.blkc({k.decl(x, N(k, "1")), k.decl(y, N(k, "2")), k.decl(p, k.addr(x)), k.decl(q, k.addr(y)), k.store(target, inner), k.ret(k.v(x))});
    k.done(f);
  });

  // ---- item 2: never in an annotation ----
  rc("seq_rej_cell_assignment_in_check", "a side effect (an assignment, increment or decrement used as a value)", [=](Kit& k) {
    Function* f = k.fn("f", k.vd);
    Var* x = k.cell(f, "x", k.i32);
    f->body = k.blkc({k.decl(x, N(k, "0")), k.chk(k.eq_c(k.asgE(x, N(k, "1")), k.i32, "1"))});
    k.done(f);
  });
  rc("seq_rej_cell_postfix_in_check", "a side effect (an assignment, increment or decrement used as a value)", [=](Kit& k) {
    Function* f = k.fn("f", k.vd);
    Var* x = k.cell(f, "x", k.i32);
    f->body = k.blkc({k.decl(x, N(k, "0")), k.chk(k.eq_c(k.incE(x), k.i32, "0"))});
    k.done(f);
  });
  rc("seq_rej_store_value_in_check", "a side effect (a write through a pointer used as a value)", [=](Kit& k) {
    Function* f = k.fn("f", k.vd);
    Var* x = k.cell(f, "x", k.i32);
    Var* p = k.local(f, "p", PT(k));
    const Expr* se = k.b.store_expr(k.pn(p), N(k, "1"), k.at());
    f->body = k.blkc({k.decl(x, N(k, "0")), k.decl(p, k.addr(x)), k.chk(k.eq_c(se, k.i32, "1"))});
    k.done(f);
  });
  rc("seq_rej_store_postfix_in_ensures", "a side effect (a write through a pointer used as a value)", [=](Kit& k) {
    Function* f = k.fn("f", k.i32, {{"p", PT(k)}});
    k.ens(f, k.eq_c(idstore_e(k, k.F(f, 0), true, true), k.i32, "0"));
    f->body = k.blk({k.ret(N(k, "0"))});
    k.done(f);
  });
}

}  // namespace

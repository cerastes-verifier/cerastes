// (included by selftest.cpp) Pointers to scalars and address-taken locals (decision 0032, M4.5b1a): cells, `&x`, `*p` as a read and as a
// write, null, comparisons and truth tests, liveness, scope exits, `?:` of pointer type and the rejections of items 2, 3, 4, 9 and 10.
// PRE-REGISTERED from the record, before the lowering existed: each expectation names a category and the VIR node whose position the
// record gives (the read of `*p`, the `Store` statement, the pointer operand of a use, the closing brace of a block), so the cases test
// the position map as well.  Each rule has a case that verifies and a twin that fails for one reason (invariant 2), except the scope-exit
// check, which cannot fail in b1a (item 8): its row is required in the map.
namespace {

void define_memory() {
  auto PT = [](Kit& k) { return k.ptr(k.i32); };
  auto N = [](Kit& k, const char* v) { return k.num(k.i32, v); };
  // `f(a: int) returns int` with 0 <= a <= 100 (so that `a + 1` and `2 * a` fit).
  auto fa = [](Kit& k, Type ret) {
    Function* f = k.fn("f", ret, {{"a", k.i32}});
    k.req(f, k.range(k.F(f, 0), k.i32, "0", "100"));
    return f;
  };

  // ---- a cell written through a pointer and read by name ----
  const auto write_read = [=](Kit& k, bool wrong) -> SrcLoc {
    Function* f = k.fn("f", k.i32);
    Var* x = k.cell(f, "x", k.i32);
    Var* p = k.local(f, "p", PT(k));
    k.ens(f, k.eq_c(k.R(f), k.i32, "2"));
    const Stmt* c = k.chk(k.eq_c(k.v(x), k.i32, wrong ? "1" : "2"));
    f->body = k.blkc({k.decl(x, N(k, "1")), k.decl(p, k.addr(x)), k.store(k.pn(p), N(k, "2")), c, k.ret(k.v(x))});
    k.done(f);
    return c->loc;
  };
  lc("mem_store_then_read", "int x = 1; int *p = &x; *p = 2; check(x == 2): the write through p is the write of x", [=](Kit& k) { write_read(k, false); });
  lc("mem_store_then_read_wrong", "the same with check(x == 1), which does not hold", [=](Kit& k) { k.fail("check", write_read(k, true)); });

  // ---- a cell by name: the flag (decision 0019) ----
  lc("mem_named_cell", "int x; x = a; return x with x a cell (its address is taken somewhere): written by name, then read", [=](Kit& k) {
    Function* f = fa(k, k.i32);
    Var* x = k.cell(f, "x", k.i32);
    k.ens(f, k.eq(k.R(f), k.F(f, 0)));
    f->body = k.blkc({k.decl(x, nullptr), k.asg(x, k.pv(f, 0)), k.ret(k.v(x))});
    k.done(f);
  });
  lc("mem_named_cell_read_first", "int x; return x with x a cell: read before any write", [=](Kit& k) {
    Function* f = fa(k, k.i32);
    Var* x = k.cell(f, "x", k.i32);
    const Expr* r = k.v(x);
    f->body = k.blkc({k.decl(x, nullptr), k.ret(r)});
    k.done(f);
    k.fail("uninitialized", r->loc);
  });

  // ---- a write through a pointer initializes the cell ----
  lc("mem_store_initializes", "int x; int *p = &x; *p = a; return x: the write through p sets the flag", [=](Kit& k) {
    Function* f = fa(k, k.i32);
    Var* x = k.cell(f, "x", k.i32);
    Var* p = k.local(f, "p", PT(k));
    k.ens(f, k.eq(k.R(f), k.F(f, 0)));
    f->body = k.blkc({k.decl(x, nullptr), k.decl(p, k.addr(x)), k.store(k.pn(p), k.pv(f, 0)), k.ret(k.v(x))});
    k.done(f);
  });
  lc("mem_store_initializes_named_read_first", "int x; int *p = &x; int y = x; *p = a: x is read by name before the write", [=](Kit& k) {
    Function* f = fa(k, k.i32);
    Var* x = k.cell(f, "x", k.i32);
    Var* p = k.local(f, "p", PT(k));
    Var* y = k.local(f, "y", k.i32);
    const Expr* r = k.v(x);
    f->body = k.blkc({k.decl(x, nullptr), k.decl(p, k.addr(x)), k.decl(y, r), k.store(k.pn(p), k.pv(f, 0)), k.ret(k.v(y))});
    k.done(f);
    k.fail("uninitialized", r->loc);
  });
  lc("mem_store_initializes_deref_first", "int x; int *p = &x; int y = *p; *p = a: x is read through p before the write", [=](Kit& k) {
    Function* f = fa(k, k.i32);
    Var* x = k.cell(f, "x", k.i32);
    Var* p = k.local(f, "p", PT(k));
    Var* y = k.local(f, "y", k.i32);
    const Expr* d = k.deref(k.pn(p));
    f->body = k.blkc({k.decl(x, nullptr), k.decl(p, k.addr(x)), k.decl(y, d), k.store(k.pn(p), k.pv(f, 0)), k.ret(k.v(y))});
    k.done(f);
    k.fail("uninitialized", d->loc);
  });

  // ---- *p as a read ----
  lc("mem_deref_read", "int x = a; int *p = &x; return *p", [=](Kit& k) {
    Function* f = fa(k, k.i32);
    Var* x = k.cell(f, "x", k.i32);
    Var* p = k.local(f, "p", PT(k));
    k.ens(f, k.eq(k.R(f), k.F(f, 0)));
    f->body = k.blkc({k.decl(x, k.pv(f, 0)), k.decl(p, k.addr(x)), k.ret(k.deref(k.pn(p)))});
    k.done(f);
  });
  lc("mem_deref_read_wrong", "the same with the postcondition r == a + 1", [=](Kit& k) {
    Function* f = fa(k, k.i32);
    Var* x = k.cell(f, "x", k.i32);
    Var* p = k.local(f, "p", PT(k));
    k.ens(f, k.eq(k.R(f), k.add(k.F(f, 0), N(k, "1"))));
    const Stmt* r = k.ret(k.deref(k.pn(p)));
    f->body = k.blkc({k.decl(x, k.pv(f, 0)), k.decl(p, k.addr(x)), r});
    k.done(f);
    k.fail("postcondition", r->loc);
  });

  // ---- null: a guard, and the dereferences it protects ----
  // int x = 0; int *p = NULL; if (a != 0) p = &x; if (p) *p = 7 (guarded) | *p = 7 | return *p; return x;
  enum class Use { Guarded, StoreUnguarded, ReadUnguarded };
  const auto null_use = [=](Kit& k, Use use) -> SrcLoc {
    Function* f = k.fn("f", k.i32, {{"a", k.i32}});
    Var* x = k.cell(f, "x", k.i32);
    Var* p = k.local(f, "p", PT(k));
    SrcLoc where;
    const Stmt* use_stmt = nullptr;
    if (use == Use::Guarded) {
      k.ens(f, k.eq(k.R(f), k.cond(k.ne(k.F(f, 0), N(k, "0")), N(k, "7"), N(k, "0"))));
      use_stmt = k.iff(k.tb(k.pn(p)), k.blk({k.store(k.pn(p), N(k, "7"))}));
    } else if (use == Use::StoreUnguarded) {
      use_stmt = k.store(k.pn(p), N(k, "7"));
      where = use_stmt->loc;
    } else {
      const Expr* d = k.deref(k.pn(p));
      use_stmt = k.ret(d);
      where = d->loc;
    }
    std::vector<const Stmt*> body = {k.decl(x, N(k, "0")), k.decl(p, k.nul(PT(k))),
                                     k.iff(k.ne(k.pv(f, 0), N(k, "0")), k.blk({k.asg(p, k.addr(x))})), use_stmt};
    if (use != Use::ReadUnguarded) body.push_back(k.ret(k.v(x)));
    f->body = k.blkc(body);
    k.done(f);
    return where;
  };
  lc("mem_null_guarded", "int *p = NULL; if (a != 0) p = &x; if (p) *p = 7: the guard makes the write safe", [=](Kit& k) { null_use(k, Use::Guarded); });
  lc("mem_null_store_unguarded", "the write *p = 7 without the guard: p may be null", [=](Kit& k) { k.fail("null-dereference", null_use(k, Use::StoreUnguarded)); });
  lc("mem_null_read_unguarded", "return *p without the guard: p may be null", [=](Kit& k) { k.fail("null-dereference", null_use(k, Use::ReadUnguarded)); });

  // ---- ?: of pointer type ----
  const auto cond_ptr = [=](Kit& k, bool guarded) -> SrcLoc {
    Function* f = k.fn("f", k.i32, {{"a", k.i32}});
    Var* x = k.cell(f, "x", k.i32);
    Var* p = k.local(f, "p", PT(k));
    SrcLoc where;
    const Stmt* use_stmt = nullptr;
    if (guarded) {
      k.ens(f, k.eq(k.R(f), k.cond(k.ne(k.F(f, 0), N(k, "0")), N(k, "1"), N(k, "0"))));
      use_stmt = k.iff(k.tb(k.pn(p)), k.blk({k.store(k.pn(p), N(k, "1"))}));
    } else {
      use_stmt = k.store(k.pn(p), N(k, "1"));
      where = use_stmt->loc;
    }
    f->body = k.blkc({k.decl(x, N(k, "0")), k.decl(p, k.cond(k.ne(k.pv(f, 0), N(k, "0")), k.addr(x), k.nul(PT(k)))), use_stmt, k.ret(k.v(x))});
    k.done(f);
    return where;
  };
  lc("mem_cond_pointer", "int *p = a != 0 ? &x : NULL; if (p) *p = 1", [=](Kit& k) { cond_ptr(k, true); });
  lc("mem_cond_pointer_unguarded", "the same without the guard: the ?: may give null", [=](Kit& k) { k.fail("null-dereference", cond_ptr(k, false)); });

  // ---- a pointer variable without an initializer (decision 0019): its flag ----
  const auto ptr_uninit = [=](Kit& k, bool both) -> SrcLoc {
    Function* f = k.fn("f", k.i32, {{"a", k.i32}});
    Var* x = k.cell(f, "x", k.i32);
    Var* y = k.cell(f, "y", k.i32);
    Var* p = k.local(f, "p", PT(k));
    const Expr* use = k.pn(p);
    std::vector<const Stmt*> body = {k.decl(x, N(k, "1")), k.decl(y, N(k, "2")), k.decl(p, nullptr)};
    if (both) {
      k.ens(f, k.eq(k.R(f), k.cond(k.ne(k.F(f, 0), N(k, "0")), N(k, "1"), N(k, "2"))));
      body.push_back(k.iff(k.ne(k.pv(f, 0), N(k, "0")), k.blk({k.asg(p, k.addr(x))}), k.blk({k.asg(p, k.addr(y))})));
    } else {
      body.push_back(k.iff(k.ne(k.pv(f, 0), N(k, "0")), k.blk({k.asg(p, k.addr(x))})));
    }
    body.push_back(k.ret(k.deref(use)));
    f->body = k.blkc(body);
    k.done(f);
    return use->loc;
  };
  lc("mem_pointer_assigned_on_all_paths", "int *p; if (a) p = &x; else p = &y; return *p", [=](Kit& k) { ptr_uninit(k, true); });
  lc("mem_pointer_unassigned_path", "int *p; if (a) p = &x; return *p: p is unassigned when a == 0", [=](Kit& k) { k.fail("uninitialized", ptr_uninit(k, false)); });

  // ---- the lifetime of a block's cells: a pointer to the inner block's cell ----
  // int x = a; int *p = &x; { int y = 1; p = &y; <inside> } <after>
  enum class In { Compare, None };
  enum class After { None, ReadX, CompareX, Copy, Truth, Store, ReadValueUsed };
  const auto inner = [=](Kit& k, In in, After after) -> SrcLoc {
    Function* f = fa(k, k.i32);
    Var* x = k.cell(f, "x", k.i32);
    Var* p = k.local(f, "p", PT(k));
    Var* y = k.cell(f, "y", k.i32);
    std::vector<const Stmt*> inside = {k.decl(y, N(k, "1")), k.asg(p, k.addr(y))};
    if (in == In::Compare) {
      // if (p == &y) x = x + *p;
      inside.push_back(k.iff(k.eq(k.pn(p), k.addr(y)), k.blk({k.asg(x, k.add(k.v(x), k.deref(k.pn(p))))})));
    }
    std::vector<const Stmt*> body = {k.decl(x, k.pv(f, 0)), k.decl(p, k.addr(x)), k.blkc(inside)};
    SrcLoc where;
    switch (after) {
      case After::None: break;
      case After::ReadX: {  // x = x + *p
        const Expr* d = k.deref(k.pn(p));
        body.push_back(k.asg(x, k.add(k.v(x), d)));
        where = d->loc;
        break;
      }
      case After::CompareX: {  // if (p == &x) x = x + 1
        const Expr* u = k.pn(p);
        body.push_back(k.iff(k.eq(u, k.addr(x)), k.blk({k.asg(x, k.add(k.v(x), N(k, "1")))})));
        where = u->loc;
        break;
      }
      case After::Copy: {  // int *q = p; if (q == &x) x = x + 1
        Var* q = k.local(f, "q", PT(k));
        const Expr* u = k.pn(p);
        body.push_back(k.decl(q, u));
        body.push_back(k.iff(k.eq(k.pn(q), k.addr(x)), k.blk({k.asg(x, k.add(k.v(x), N(k, "1")))})));
        where = u->loc;
        break;
      }
      case After::Truth: {  // if (p) x = x + 1
        const Expr* u = k.pn(p);
        body.push_back(k.iff(k.tb(u), k.blk({k.asg(x, k.add(k.v(x), N(k, "1")))})));
        where = u->loc;
        break;
      }
      case After::Store: {  // *p = 1;
        const Stmt* s = k.store(k.pn(p), N(k, "1"));
        body.push_back(s);
        where = s->loc;
        break;
      }
      case After::ReadValueUsed: {  // int z = *p + 1; return z (the value of the failed read is used)
        Var* z = k.local(f, "z", k.i32);
        const Expr* d = k.deref(k.pn(p));
        body.push_back(k.decl(z, k.add(d, N(k, "1"))));
        body.push_back(k.ret(k.v(z)));
        where = d->loc;
        break;
      }
    }
    if (after != After::ReadValueUsed) {
      k.ens(f, k.eq(k.R(f), k.add(k.F(f, 0), N(k, "1"))));
      body.push_back(k.ret(k.v(x)));
    }
    f->body = k.blkc(body);
    k.done(f);
    return where;
  };
  // The postcondition r == a + 1 holds only when the block's work adds 1 to x: the positive adds *p (= 1) inside the block; the twins that
  // read or compare after the block add 1 there too (CompareX, Copy, Truth) or are not meant to reach it (the failure comes first).
  lc("mem_inner_block_inside", "{ int y = 1; p = &y; if (p == &y) x = x + *p; }: p is used (compared, read) inside the block", [=](Kit& k) { inner(k, In::Compare, After::None); });
  lc("mem_inner_block_read_after", "x = x + *p after the block: y's block has ended, the permission is gone", [=](Kit& k) {
    k.fail("permission", inner(k, In::None, After::ReadX));
  });
  lc("mem_inner_block_compare_after", "if (p == &x) after the block: the value of p is used after y ended", [=](Kit& k) {
    k.fail("lifetime", inner(k, In::None, After::CompareX));
  });
  lc("mem_inner_block_copy_after", "int *q = p after the block: a copy of a dangling pointer value", [=](Kit& k) {
    k.fail("lifetime", inner(k, In::None, After::Copy));
  });
  lc("mem_inner_block_truth_after", "if (p) after the block: a truth test of a dangling pointer", [=](Kit& k) {
    k.fail("lifetime", inner(k, In::None, After::Truth));
  });
  lc("mem_inner_block_store_after", "*p = 1 after the block: a write to y, whose block has ended", [=](Kit& k) {
    k.fail("permission", inner(k, In::None, After::Store));
  });
  lc("mem_inner_block_read_value_used", "int z = *p + 1; return z after the block: one error, the permission, though z is used afterwards", [=](Kit& k) {
    k.fail("permission", inner(k, In::None, After::ReadValueUsed));
  });

  // ---- scope exits: fall-through and return, nested blocks with cells ----
  // The scope-exit check (item 8) cannot fail in b1a: its row must exist, at the closing brace of the block.
  const auto scopes = [=](Kit& k, bool wrong) -> SrcLoc {
    Function* f = k.fn("f", k.i32, {{"a", k.i32}});
    k.req(f, k.range(k.F(f, 0), k.i32, "1", "999"));
    k.ens(f, k.eq(k.R(f), k.add(k.F(f, 0), N(k, "3"))));
    Var* x = k.cell(f, "x", k.i32);
    Var* p = k.local(f, "p", PT(k));
    Var* w = k.cell(f, "w", k.i32);
    Var* pw = k.local(f, "pw", PT(k));
    Var* y = k.cell(f, "y", k.i32);
    Var* q = k.local(f, "q", PT(k));
    Var* z = k.cell(f, "z", k.i32);
    Var* s = k.local(f, "s", PT(k));
    const Stmt* fall = k.blkc({k.decl(w, N(k, "1")), k.decl(pw, k.addr(w)), k.store(k.pn(pw), N(k, "3")),
                               k.asg(x, k.add(k.v(x), k.deref(k.pn(pw))))});
    const Stmt* inner_ret = k.ret(k.deref(k.pn(s)));
    const Expr* zinit = k.deref(k.pn(q));
    const Stmt* innermost = k.blkc({k.decl(z, wrong ? k.add(zinit, N(k, "1")) : zinit), k.decl(s, k.addr(z)),
                                    k.iff(k.gt(k.deref(k.pn(s)), N(k, "0")), k.blk({inner_ret}))});
    const Stmt* big = k.blkc({k.decl(y, k.deref(k.pn(p))), k.decl(q, k.addr(y)), innermost, k.ret(k.v(y))});
    f->body = k.blkc({k.decl(x, k.pv(f, 0)), k.decl(p, k.addr(x)), fall,
                      k.iff(k.gt(k.pv(f, 0), N(k, "500")), big), k.ret(k.v(x))});
    k.done(f);
    k.require_map("lifetime", "may still be in use when its lifetime ends");
    return inner_ret->loc;
  };
  lc("mem_scope_exits", "a fall-through exit and returns from nested blocks that hold cells: every exit gives its cells back", [=](Kit& k) { scopes(k, false); });
  lc("mem_scope_exits_wrong", "z one too many: the return from the innermost block breaks the postcondition", [=](Kit& k) {
    k.fail("postcondition", scopes(k, true));
  });

  // ---- a goto past a cell's declaration (allocation at block entry, item 4) ----
  const auto goto_past = [=](Kit& k, bool read_first) -> SrcLoc {
    Function* f = k.fn("f", k.i32, {{"a", k.i32}});
    Var* y = k.cell(f, "y", k.i32);
    Var* x = k.cell(f, "x", k.i32);
    Var* q = k.local(f, "q", PT(k));
    const int id = k.jid();
    SrcLoc where;
    std::vector<const Stmt*> body = {k.decl(y, N(k, "0")), k.iff(k.gt(k.pv(f, 0), N(k, "5")), k.blk({k.go(id)})), k.decl(x, nullptr),
                                     k.decl(q, nullptr), k.asg(q, k.addr(x)), k.store(k.pn(q), N(k, "3")), k.asg(y, k.v(x)), k.lab(id)};
    if (read_first) {
      k.ens(f, k.lor(k.gt(k.F(f, 0), N(k, "5")), k.eq_c(k.R(f), k.i32, "6")));
      const Expr* rx = k.v(x);
      where = rx->loc;
      body.push_back(k.ret(k.add(k.v(y), rx)));
    } else {
      k.ens(f, k.eq(k.R(f), k.cond(k.gt(k.F(f, 0), N(k, "5")), N(k, "7"), N(k, "10"))));
      body.push_back(k.asg(x, N(k, "7")));
      body.push_back(k.ret(k.add(k.v(y), k.v(x))));
    }
    f->body = k.blkc(body);
    k.done(f);
    return where;
  };
  lc("mem_goto_past_cell", "a goto past the declaration of a cell: at the label the cell is allocated and uninitialized; it is written, then read", [=](Kit& k) { goto_past(k, false); });
  lc("mem_goto_past_cell_read", "the label reads the cell, which the goto's path never wrote", [=](Kit& k) { k.fail("uninitialized", goto_past(k, true)); });

  // ---- *p op= k, ++*p ----
  const auto bump = [=](Kit& k, bool guarded) -> SrcLoc {
    Function* f = k.fn("f", k.i32, {{"a", k.i32}});
    k.req(f, k.ge_c(k.F(f, 0), k.i32, "0"));
    k.ens(f, k.ge(k.R(f), k.F(f, 0)));
    Var* x = k.cell(f, "x", k.i32);
    Var* p = k.local(f, "p", PT(k));
    const Stmt* inc = k.idstore(k.pn(p), true);
    const Stmt* use = guarded ? k.iff(k.lt(k.deref(k.pn(p)), N(k, "2147483647")), k.blk({inc})) : inc;
    f->body = k.blkc({k.decl(x, k.pv(f, 0)), k.decl(p, k.addr(x)), use, k.ret(k.v(x))});
    k.done(f);
    return inc->loc;
  };
  lc("mem_increment_guarded", "if (*p < INT_MAX) ++*p", [=](Kit& k) { bump(k, true); });
  lc("mem_increment_unguarded", "++*p without the guard overflows at INT_MAX, reported at the store", [=](Kit& k) { k.fail("overflow", bump(k, false)); });
  const auto add_to = [=](Kit& k, const char* hi) -> SrcLoc {
    Function* f = k.fn("f", k.i32, {{"a", k.i32}});
    k.req(f, k.range(k.F(f, 0), k.i32, "0", hi));
    k.ens(f, k.ge(k.R(f), k.F(f, 0)));
    Var* x = k.cell(f, "x", k.i32);
    Var* p = k.local(f, "p", PT(k));
    const Stmt* add = k.cstore(k.pn(p), BinaryOp::Add, k.pv(f, 0));
    f->body = k.blkc({k.decl(x, k.pv(f, 0)), k.decl(p, k.addr(x)), add, k.ret(k.v(x))});
    k.done(f);
    return add->loc;
  };
  lc("mem_add_through_pointer", "*p += a with 0 <= a <= 2^30 - 1: 2a fits", [=](Kit& k) { add_to(k, "1073741823"); });
  lc("mem_add_through_pointer_wide", "*p += a with a up to 2^30: 2a does not fit, reported at the store", [=](Kit& k) { k.fail("overflow", add_to(k, "1073741824")); });

  // ---- comparisons: == and != of pointers, null in the spellings of VIR (a Null of any pointer type) ----
  const auto compares = [=](Kit& k, bool wrong) -> SrcLoc {
    Function* f = k.fn("f", k.i32);
    k.ens(f, k.eq_c(k.R(f), k.i32, wrong ? "5" : "4"));
    Var* x = k.cell(f, "x", k.i32);
    Var* y = k.cell(f, "y", k.i32);
    Var* p = k.local(f, "p", PT(k));
    Var* a = k.local(f, "a", PT(k));
    Var* n = k.local(f, "n", k.i32);
    const Stmt* c = k.chk(k.ne(k.addr(x), k.addr(y)));
    const Stmt* r = k.ret(k.v(n));
    auto count = [&](const Expr* cnd) { return k.iff(cnd, k.blk({k.asg(n, k.add(k.v(n), N(k, "1")))})); };
    f->body = k.blkc({k.decl(x, N(k, "1")), k.decl(y, N(k, "2")), k.decl(p, k.addr(x)), k.decl(a, k.nul(PT(k))), k.decl(n, N(k, "0")), c,
                      count(k.eq(k.pn(p), k.addr(x))), count(k.ne(k.pn(p), k.addr(y))), count(k.eq(k.pn(a), k.nul(PT(k)))),
                      count(k.lnot(k.tb(k.pn(a)))), r});
    k.done(f);
    return r->loc;
  };
  lc("mem_pointer_comparisons", "&x != &y, p == &x, p != &y, a == NULL, !a: four tests count, check(&x != &y) holds", [=](Kit& k) { compares(k, false); });
  lc("mem_pointer_comparisons_wrong", "the same with the postcondition r == 5", [=](Kit& k) { k.fail("postcondition", compares(k, true)); });
  lc("mem_address_check_wrong", "check(&x == &y): the addresses of two different cells are never equal", [=](Kit& k) {
    Function* f = k.fn("f", k.vd);
    Var* x = k.cell(f, "x", k.i32);
    Var* y = k.cell(f, "y", k.i32);
    const Stmt* c = k.chk(k.eq(k.addr(x), k.addr(y)));
    f->body = k.blkc({k.decl(x, N(k, "1")), k.decl(y, N(k, "2")), c});
    k.done(f);
    k.fail("check", c->loc);
  });

  // ---- pointer types: bool, a narrow unsigned type, an enumeration (one cell type each) ----
  const auto kinds = [=](Kit& k, bool set_b) -> SrcLoc {
    Function* f = k.fn("f", k.i32);
    k.ens(f, k.eq_c(k.R(f), k.i32, "3"));
    const Type e = k.en("color", 32, true, "0", "2");
    Var* b = k.cell(f, "b", k.bl);
    Var* c = k.cell(f, "c", k.u8);
    Var* g = k.cell(f, "g", e);
    Var* pb = k.local(f, "pb", k.ptr(k.bl));
    Var* pc = k.local(f, "pc", k.ptr(k.u8));
    Var* pg = k.local(f, "pg", k.ptr(e));
    Var* n = k.local(f, "n", k.i32);
    std::vector<const Stmt*> body = {k.decl(b, k.no()), k.decl(c, k.num(k.u8, "0")), k.decl(g, k.num(e, "0")), k.decl(pb, k.addr(b)),
                                     k.decl(pc, k.addr(c)), k.decl(pg, k.addr(g))};
    if (set_b) body.push_back(k.store(k.pn(pb), k.yes()));
    body.push_back(k.store(k.pn(pc), k.num(k.u8, "2")));
    body.push_back(k.store(k.pn(pg), k.num(e, "1")));
    body.push_back(k.decl(n, k.num(k.i32, "0")));
    body.push_back(k.iff(k.deref(k.pn(pb)), k.blk({k.asg(n, k.add(k.v(n), N(k, "1")))})));
    body.push_back(k.asg(n, k.add(k.v(n), k.cv(k.i32, k.deref(k.pn(pc))))));
    const Stmt* r = k.ret(k.v(n));
    body.push_back(r);
    f->body = k.blkc(body);
    k.done(f);
    return r->loc;
  };
  lc("mem_pointer_kinds", "pointers to a bool, an unsigned char and an enumeration: each cell has its own field", [=](Kit& k) { kinds(k, true); });
  lc("mem_pointer_kinds_wrong", "the bool is never set, so the sum is 2, not 3", [=](Kit& k) { k.fail("postcondition", kinds(k, false)); });
}

void define_memory_rejections() {
  auto PT = [](Kit& k) { return k.ptr(k.i32); };
  auto N = [](Kit& k, const char* v) { return k.num(k.i32, v); };

  // ---- item 2: pointer types ----
  rc("mem_rej_pointer_to_pointer", "a pointer to a pointer", [=](Kit& k) {
    Function* f = k.fn("f", k.vd);
    Var* pp = k.local(f, "pp", k.ptr(PT(k)));
    f->body = k.blk({k.decl(pp, nullptr)});
    k.done(f);
  });
  rc("mem_rej_pointer_to_void", "a pointer to void", [=](Kit& k) {
    Function* f = k.fn("f", k.vd);
    Var* v = k.local(f, "v", k.ptr(k.vd));
    f->body = k.blk({k.decl(v, nullptr)});
    k.done(f);
  });

  // ---- item 3: conversions ----
  rc("mem_rej_pointer_conversion", "a conversion between", [=](Kit& k) {
    Function* f = k.fn("f", k.vd);
    Var* x = k.cell(f, "x", k.i32);
    Var* l = k.local(f, "l", k.i64);
    f->body = k.blkc({k.decl(x, N(k, "0")), k.decl(l, k.cv(k.i64, k.addr(x)))});
    k.done(f);
  });
  rc("mem_rej_pointers_of_different_types", "pointers of different types", [=](Kit& k) {
    Function* f = k.fn("f", k.vd);
    Var* x = k.cell(f, "x", k.i32);
    Var* y = k.cell(f, "y", k.i64);
    Var* b = k.local(f, "b", k.bl);
    f->body = k.blkc({k.decl(x, N(k, "0")), k.decl(y, k.num(k.i64, "0")), k.decl(b, k.eq(k.addr(x), k.addr(y)))});
    k.done(f);
  });

  // ---- item 4: addresses ----
  rc("mem_rej_address_of_pointer_variable", "the address of the pointer variable", [=](Kit& k) {
    Function* f = k.fn("f", k.vd);
    Var* p = k.local(f, "p", PT(k));
    f->mark_address_taken(p);
    f->body = k.blk({k.decl(p, k.nul(PT(k)))});
    k.done(f);
  });
  rc("mem_rej_address_of_unmarked_variable", "internal: the address of 'x', which is not marked address-taken", [=](Kit& k) {
    Function* f = k.fn("f", k.vd);
    Var* x = k.local(f, "x", k.i32);
    Var* p = k.local(f, "p", PT(k));
    f->body = k.blk({k.decl(x, N(k, "0")), k.decl(p, k.addr(x))});
    k.done(f);
  });
  rc("mem_rej_dereference_of_an_integer", "a dereference of a", [=](Kit& k) {
    Function* f = k.fn("f", k.i32);
    f->body = k.blk({k.ret(k.deref(N(k, "0")))});
    k.done(f);
  });
  rc("mem_rej_store_of_the_wrong_type", "a store of a", [=](Kit& k) {
    Function* f = k.fn("f", k.vd);
    Var* x = k.cell(f, "x", k.i32);
    Var* p = k.local(f, "p", PT(k));
    f->body = k.blkc({k.decl(x, N(k, "0")), k.decl(p, k.addr(x)), k.store(k.pn(p), k.yes())});
    k.done(f);
  });

  // ---- item 9: a cell is written only as the top-level operation of a statement: lifted by decision 0039 item 2 (M4.5b1c2); the two cases that
  // rejected `y = (x = 1)` and `y = x++` on a cell are the lowering cases seq_cell_assign_value and seq_cell_postfix_value (selftest_cases10.inc) ----

  // ---- item 10: a block without its closing brace (the loops and the jumps of item 10 are lowering cases since decision 0038: selftest_cases9.inc) ----
  rc("mem_rej_cell_block_without_a_closing_brace", "has no closing-brace position", [=](Kit& k) {
    Function* f = k.fn("f", k.vd);
    Var* x = k.cell(f, "x", k.i32);
    f->body = k.blk({k.decl(x, N(k, "0"))});
    k.done(f);
  });
  rc("mem_rej_switch_body_without_a_closing_brace", "the body of a switch declares a cell but has no closing-brace position", [=](Kit& k) {
    Function* f = k.fn("f", k.vd, {{"code", k.i32}});
    Var* x = k.cell(f, "x", k.i32);
    const int id = k.jid();
    const Stmt* body = k.blk({k.decl(x, nullptr), k.cs(N(k, "0")), k.asg(x, N(k, "1"))});
    f->body = k.blk({k.sw(k.pv(f, 0), body, id)});
    k.done(f);
  });
}

}  // namespace

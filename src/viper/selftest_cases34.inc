// (included by selftest.cpp) Arrays in structures and in static storage, M4.5e1e1 (decision 0063).  Part A (plumbing): `Type::Array` is representable as the type of a StructMember and
// of a Global; the three views of a struct's leaves (struct_leaves, the old view, keeps the array member as one leaf of the array's type: the poison; struct_object_leaves gives an
// array leaf; struct_value_leaves one leaf per element); `Global::elems` and `GlobalLeaf::elems` hold the initial values of an array; the statements `ArrayZero` and `ArrayCopy` and the
// expression `ElemValue` are built by the Builder.  Part B (a global array: define_array_globals) and part C (an array member: define_member_arrays) accept the array member, the
// element of an array member, `owns(p->a)`, `ArrayZero` and the struct global's array leaf; what check_program still refuses (a by-value structure with an array member: part E; the
// array copy and `ElemValue`; questions 292-294 and 306) has its rejection cases in the first function below and in define_member_arrays, and lowering stops a refused program that
// reaches it anyway with an internal error (the `ru` cases, which lower a refused program).
namespace {

void define_arrays_in_storage_rejections()
{
    auto N = [](Kit& k, const char* v)
    {
        return k.num(k.i32, v);
    };
    auto A3 = [](Kit& k)
    {
        return k.prog.array_of(k.i32, 3U);
    };
    // struct S { int a[3]; int n; }
    auto S_arr = [=](Kit& k)
    {
        return k.strukt("S", {{"a", A3(k)}, {"n", k.i32}});
    };

    // ---- an array member of a struct ----
    // (the member that is an array of structures, refused here until M4.5e1e2 Part C2, is accepted by check_program since its slice 1: e1e2c2_member_*)

    // ---- a global or `static` array: what part B still refuses ----
    rc("arr_store_rej_global_array_zero_length", "of no elements",
       [=](Kit& k)
       {
           // int g[0];
           k.prog.add_global("g", k.prog.array_of(k.i32, 0U), k.at(), nullptr);
       });
    rc("arr_store_rej_global_array_of_arrays", "decision 0059 items 1 and 2",
       [=](Kit& k)
       {
           // int g[2][3]; (M4.5e1e2)
           k.prog.add_global("g", k.prog.array_of(A3(k), 2U), k.at(), nullptr);
       });
    rc("arr_store_rej_global_array_owning", "an array with `owning` or `ref_to_uninit`",
       [=](Kit& k)
       {
           Global* g {k.prog.add_global("g", k.prog.array_of(k.ptr(k.i32), 2U), k.at(), nullptr)};
           g->owning = true;
       });
    rc("arr_store_rej_global_array_too_many_values", "lists 4 initial values",
       [=](Kit& k)
       {
           // int g[3] = {1, 2, 3, 4}: more than the elements
           Global* g {k.prog.add_global("g", A3(k), k.at(), nullptr)};
           GlobalLeaf one;
           one.value = "1";
           g->elems = {one, one, one, one};
       });
    rc("arr_store_rej_global_array_unknown_element_zero_rest",
       "unknown initial value although the rest of the array is zero",
       [=](Kit& k)
       {
           Global* g {k.prog.add_global("g", A3(k), k.at(), nullptr)};
           GlobalLeaf one;
           one.kind = GlobalInit::Unknown;
           g->elems = {one};
       });
    rc("arr_store_rej_global_array_known_element_unknown_rest",
       "known initial value although the rest of the array is not known",
       [=](Kit& k)
       {
           Global* g {k.prog.add_global("g", A3(k), k.at(), nullptr)};
           GlobalLeaf one;
           one.value = "1";
           g->elems = {one};
           g->rest = ArrayRest::Unknown;
       });
    rc("arr_store_rej_atom_on_part_of_a_global_array", "whose count is not the literal number of its elements",
       [=](Kit& k)
       {
           // owns_range(g, 2) of int g[3]: a part of a global array is M4.5e2's
           Global* g {k.prog.add_global("g", A3(k), k.at(), nullptr)};
           Function* f {k.fn("f", k.vd)};
           k.req(f, k.b.range_atom(AtomKind::OwnsRange, k.b.global_addr(g, k.at()), N(k, "2"), 0, k.at()));
           f->body = k.blk({});
           k.done(f);
       });
    rc("arr_store_rej_element_read_of_a_static_local_array_in_a_contract", "its name is not in scope there",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.vd)};
           Global* g {k.prog.add_global("s", A3(k), k.at(), f)};
           k.req(f, k.eq(k.deref(k.b.elem_addr(k.b.global_addr(g, k.at()), N(k, "1"), k.at())), N(k, "0")));
           f->body = k.blk({});
           k.done(f);
       });
    rc("arr_store_rej_scalar_global_with_array_values", "carries array initial values",
       [=](Kit& k)
       {
           // an `int g;` that carries the initial values of an array
           Global* g {k.prog.add_global("g", k.i32, k.at(), nullptr)};
           GlobalLeaf one;
           one.value = "1";
           g->elems = {one};
       });
    rc("arr_store_rej_struct_global_leaf_with_array_values", "carries array initial values",
       [=](Kit& k)
       {
           // struct S { int x; } g; whose leaf x carries the initial values of an array
           StructType* S {k.strukt("S", {{"x", k.i32}})};
           Global* g {k.prog.add_global("g", k.st(S), k.at(), nullptr)};
           GlobalLeaf l;
           GlobalLeaf one;
           one.value = "1";
           l.elems = {one};
           g->leaves = {l};
       });

    // ---- an element of such an array (ElemAddr over GlobalAddr and MemberAddr) ----

    // ---- the new statements and the new expression ----
    rc("arr_store_rej_array_zero_of_a_local_array",
       "an array zero-fill of something other than the address of an array member",
       [=](Kit& k)
       {
           // even the address of an array local (whose zero fill is Var::zero_filled) is not an ArrayZero's operand yet
           Function* f {k.fn("f", k.vd)};
           Var* a {k.cell(f, "a", A3(k))};
           f->body = k.blkc({k.decl(a, nullptr), k.b.array_zero(k.addr(a), k.at())});
           k.done(f);
       });
    rc("arr_store_rej_array_copy_of_a_non_array",
       "an array copy of something other than the addresses of two array members",
       [=](Kit& k)
       {
           // the operands of an array copy are the addresses of array members (decision 0063 item 13)
           StructType* S {S_arr(k)};
           Function* f {k.fn("f", k.vd)};
           Var* s {k.scell(f, "s", S)};
           Var* t {k.scell(f, "t", S)};
           f->body = k.blkc({k.decl(s, nullptr), k.decl(t, nullptr), k.b.array_copy(k.addr(t), k.addr(s), k.at())});
           k.done(f);
       });
    rc("arr_store_rej_array_copy_of_two_types", "an array copy between two array members of different types",
       [=](Kit& k)
       {
           // struct U { int a[4]; long b[4]; }: u.a into u.b
           StructType* U {k.strukt("U", {{"a", k.prog.array_of(k.i32, 4U)}, {"b", k.prog.array_of(k.i64, 4U)}})};
           Function* f {k.fn("f", k.vd)};
           Var* u {k.scell(f, "u", U)};
           f->body = k.blkc({k.decl(u, nullptr), k.b.array_copy(k.mem(k.addr(u), 1), k.mem(k.addr(u), 0), k.at())});
           k.done(f);
       });
    // a function that returns a struct with an array member, and the element of its result
    rc("arr_store_rej_elem_value_of_a_cell_member", "is not an array member",
       [=](Kit& k)
       {
           // int f(void) { return mk().n[1]; }: member 1 of the structure is a cell
           StructType* S {S_arr(k)};
           Function* mk {k.fn("mk", k.st(S))};
           k.trusted(mk);
           Function* f {k.fn("f", k.i32)};
           f->body = k.blk({k.ret(k.b.elem_value(k.call(mk, {}), 1, N(k, "1"), k.at()))});
           k.done(f);
       });
    rc("arr_store_rej_elem_value_with_a_pointer_index",
       "the index of an element of a struct value's array member is not an integer",
       [=](Kit& k)
       {
           StructType* S {S_arr(k)};
           Function* mk {k.fn("mk", k.st(S))};
           k.trusted(mk);
           Function* f {k.fn("f", k.i32)};
           f->body = k.blk({k.ret(k.b.elem_value(k.call(mk, {}), 0, k.nul(k.ptr(k.i32)), k.at()))});
           k.done(f);
       });
}

// A global or `static` array of scalars (decision 0063 part B: items 2, 3, 5, 7-11).  The block `GB_g()` of N slots: `program_start`'s row group (the two quantified permissions,
// the flags, the listed values and the zero rest or the unknown range, the liveness cell), `main` holding it, `g[i]` read and written by name over the bound local, `owns(g)` and
// `reads(g)` as range atoms over the whole array, element reads in a contract, loops, and the aliasing trap.  Each rule has a case that verifies and a twin that fails for one reason.
void define_array_globals()
{
    auto N = [](Kit& k, const char* v)
    {
        return k.num(k.i32, v);
    };
    [[maybe_unused]] auto PI = [](Kit& k)
    {
        return k.ptr(k.i32);
    };
    [[maybe_unused]] auto FP = [](Kit& k, Function* f, int i)
    {
        return k.b.text(k.F(f, i), f->params[static_cast<size_t>(i)].name);
    };
    // `int <name>[n] = {vals...}` with the rest zero (`unknown`: the value of no element is known)
    auto garr = [](Kit& k, const char* name, unsigned long long n, const std::vector<const char*>& vals,
                   bool unknown = false, const Function* owner = nullptr) -> Global*
    {
        Global* g {k.prog.add_global(name, k.prog.array_of(k.i32, n), k.at(), owner)};
        for (const char* v : vals)
        {
            GlobalLeaf l;
            l.kind = unknown ? GlobalInit::Unknown : GlobalInit::Value;
            l.value = v;
            l.init = k.num(k.i32, v);
            g->elems.push_back(l);
        }
        g->rest = unknown ? ArrayRest::Unknown : ArrayRest::Zero;
        return g;
    };
    auto ga = [](Kit& k, const Global* g)
    {
        return k.b.global_addr(g, k.at());
    };
    // `g[i]`: the address of the element, a read, a store
    auto gat = [=](Kit& k, const Global* g, const Expr* i)
    {
        return k.b.elem_addr(ga(k, g), i, k.at());
    };
    auto grd = [=](Kit& k, const Global* g, const Expr* i)
    {
        return k.deref(gat(k, g, i));
    };
    auto gwr = [=](Kit& k, const Global* g, const Expr* i, const Expr* v)
    {
        return k.store(gat(k, g, i), v);
    };
    [[maybe_unused]] auto gatom = [=](Kit& k, AtomKind kind, const Global* g, int amount, bool implicit = false)
    {
        return k.b.global_atom(kind, ga(k, g), amount, k.at(), implicit);
    };
    // a positive case and its vacuity twin (the same with check(false) last, which must fail: the program_start rows are not inconsistent)
    auto lcv = [](const char* name, const char* doc, const std::function<void(Kit&)>& build)
    {
        lc(name, doc, build);
        lc(std::string(name) + "_vacuity",
           "the same with check(false) after everything: it fails (the global array's rows are consistent)",
           [build](Kit& k)
           {
               k.vac_tail = true;
               build(k);
               k.fail("check", k.vac_loc);
           });
    };
    // the end of `main`: `return 0;`, or `check(false); return 0;` for a vacuity twin
    auto main_end = [](Kit& k, std::vector<const Stmt*>& body)
    {
        if (k.vac_tail)
        {
            const Stmt* v {k.chk(k.no())};
            body.push_back(v);
            k.vac_loc = v->loc;
        }
        body.push_back(k.ret(k.num(k.i32, "0")));
    };

    // ---- item 9: program_start; item 5: an element by name ----
    // int g[4] = {1, 2}; int main() { check(g[0] == 1 && g[1] == 2 && g[2] == 0 && g[3] == 0); g[1] = 5; check(g[1] == 5 && g[0] == 1 && g[2] == 0); return 0; }
    lcv("garr_main_read_write",
        "main holds g[4] = {1, 2} from program_start: the listed prefix, the zero rest, then a write and a read back",
        [=](Kit& k)
        {
            Global* g {garr(k, "g", 4U, {"1", "2"})};
            Function* m {k.fn("main", k.i32)};
            const Stmt* c1 {k.chk(
                k.land(k.land(k.eq_c(grd(k, g, N(k, "0")), k.i32, "1"), k.eq_c(grd(k, g, N(k, "1")), k.i32, "2")),
                       k.land(k.eq_c(grd(k, g, N(k, "2")), k.i32, "0"), k.eq_c(grd(k, g, N(k, "3")), k.i32, "0"))))};
            const Stmt* w {gwr(k, g, N(k, "1"), N(k, "5"))};
            const Stmt* c2 {k.chk(
                k.land(k.eq_c(grd(k, g, N(k, "1")), k.i32, "5"),
                       k.land(k.eq_c(grd(k, g, N(k, "0")), k.i32, "1"), k.eq_c(grd(k, g, N(k, "2")), k.i32, "0"))))};
            std::vector<const Stmt*> body {c1, w, c2};
            main_end(k, body);
            m->body = k.blk(body);
            k.done(m);
            k.prog.set_entry(m);
        });
    lc("garr_main_zero_rest_wrong", "the zero rest is exactly the elements past the list: g[3] == 1 fails",
       [=](Kit& k)
       {
           Global* g {garr(k, "g", 4U, {"1", "2"})};
           Function* m {k.fn("main", k.i32)};
           const Stmt* c {k.chk(k.eq_c(grd(k, g, N(k, "3")), k.i32, "1"))};
           m->body = k.blk({c, k.ret(N(k, "0"))});
           k.done(m);
           k.prog.set_entry(m);
           k.fail("check", c->loc);
       });
    lc("garr_main_unknown_wrong", "a global array whose values are not known: g[0] == 0 does not follow",
       [=](Kit& k)
       {
           Global* g {garr(k, "g", 3U, {}, true)};
           Function* m {k.fn("main", k.i32)};
           const Stmt* c {k.chk(k.eq_c(grd(k, g, N(k, "0")), k.i32, "0"))};
           m->body = k.blk({c, k.ret(N(k, "0"))});
           k.done(m);
           k.prog.set_entry(m);
           k.fail("check", c->loc);
       });
    lc("garr_main_index_outside", "g[4] of int g[4]: the bound row (out-of-bounds) at the read",
       [=](Kit& k)
       {
           Global* g {garr(k, "g", 4U, {"1"})};
           Function* m {k.fn("main", k.i32)};
           const Expr* r {grd(k, g, N(k, "4"))};
           const Stmt* c {k.chk(k.eq_c(r, k.i32, "0"))};
           m->body = k.blk({c, k.ret(N(k, "0"))});
           k.done(m);
           k.prog.set_entry(m);
           k.fail("out-of-bounds", r->loc);
       });
    // ---- items 7 and 8: owns(g) and reads(g) in a contract, at a call and at a return ----
    // void put() requires(owns(g)) ensures(owns(g) && g[1] == 9) { g[1] = 9; }
    auto make_put = [=](Kit& k, Global* g, bool wrong) -> Function*
    {
        Function* f {k.fn("put", k.vd)};
        k.req(f, gatom(k, AtomKind::Owns, g, 0));
        k.ens(f, k.land(gatom(k, AtomKind::Owns, g, 0), k.eq_c(grd(k, g, N(k, "1")), k.i32, "9")));
        const Stmt* w {gwr(k, g, N(k, "1"), N(k, wrong ? "8" : "9"))};
        f->body = k.blk({w});
        k.done(f);
        if (wrong)
        {
            k.fail("postcondition", f->end_loc);
        }
        return f;
    };
    // int main() { put(); check(g[1] == 9); check(g[0] == [1|2]); return 0; }
    auto put_main = [=](Kit& k, bool wrong_check)
    {
        Global* g {garr(k, "g", 4U, {"1", "2"})};
        Function* put {make_put(k, g, false)};
        Function* m {k.fn("main", k.i32)};
        const Stmt* c0 {k.es(k.call(put, {}))};
        const Stmt* c1 {k.chk(k.eq_c(grd(k, g, N(k, "1")), k.i32, "9"))};
        std::vector<const Stmt*> body {c0, c1};
        if (wrong_check)
        {
            // the callee owned the whole array and promised only g[1] == 9: g[0] == 1 is not known any more
            const Stmt* c2 {k.chk(k.eq_c(grd(k, g, N(k, "0")), k.i32, "1"))};
            body.push_back(c2);
            k.fail("check", c2->loc);
        }
        body.push_back(k.ret(N(k, "0")));
        m->body = k.blk(body);
        k.done(m);
        k.prog.set_entry(m);
    };
    lc("garr_owns_call", "main owns g from program_start: put() takes owns(g) and gives it back with g[1] == 9",
       [=](Kit& k)
       {
           put_main(k, false);
       });
    lc("garr_owns_call_forgot", "put() owns the whole of g and promises only g[1] == 9: g[0] == 1 after the call fails",
       [=](Kit& k)
       {
           put_main(k, true);
       });
    lc("garr_owns_callee_wrong", "put() writes 8 but promises g[1] == 9: the postcondition fails at the closing brace",
       [=](Kit& k)
       {
           Global* g {garr(k, "g", 4U, {"1", "2"})};
           make_put(k, g, true);
       });

    // int peek() requires(reads(g)) ensures(reads(g) && result == g[2]) { return g[2]; }  (mode 1: writes g[1] under reads(g))
    auto make_peek = [=](Kit& k, Global* g, bool writes) -> Function*
    {
        Function* f {k.fn("peek", k.i32)};
        k.req(f, gatom(k, AtomKind::Reads, g, 0));
        k.ens(f, k.land(gatom(k, AtomKind::Reads, g, 0), k.eq(k.R(f), grd(k, g, N(k, "2")))));
        std::vector<const Stmt*> body;
        if (writes)
        {
            const Stmt* w {gwr(k, g, N(k, "1"), N(k, "3"))};
            body.push_back(w);
            k.fail("permission", w->loc);
        }
        body.push_back(k.ret(grd(k, g, N(k, "2"))));
        f->body = k.blk(body);
        k.done(f);
        return f;
    };
    // int main() { int r = peek(); check(r == 0 && g[0] == 1); return 0; }
    auto peek_main = [=](Kit& k, bool writes)
    {
        Global* g {garr(k, "g", 4U, {"1", "2"})};
        Function* peek {make_peek(k, g, writes)};
        Function* m {k.fn("main", k.i32)};
        Var* r {k.local(m, "r", k.i32)};
        const Stmt* d {k.decl(r, k.call(peek, {}))};
        const Stmt* c {k.chk(k.land(k.eq_c(k.v(r), k.i32, "0"), k.eq_c(grd(k, g, N(k, "0")), k.i32, "1")))};
        m->body = k.blk({d, c, k.ret(N(k, "0"))});
        k.done(m);
        k.prog.set_entry(m);
    };
    lc("garr_reads_call", "peek() borrows g at a fraction and returns it: main reads g[0] and g[2] == 0 after",
       [=](Kit& k)
       {
           peek_main(k, false);
       });
    lc("garr_reads_write", "a write under reads(g): permission at the store (the atom is a fraction)",
       [=](Kit& k)
       {
           peek_main(k, true);
       });
    lc("garr_store_without_atom", "a function with no atom on g writes g[0]: permission at the store",
       [=](Kit& k)
       {
           Global* g {garr(k, "g", 4U, {"1"})};
           Function* f {k.fn("f", k.vd)};
           const Stmt* w {gwr(k, g, N(k, "0"), N(k, "7"))};
           f->body = k.blk({w});
           k.done(f);
           k.fail("permission", w->loc);
       });
    lc("garr_call_without_permission",
       "a function with no atom on g calls put(), which needs owns(g): permission at the call",
       [=](Kit& k)
       {
           Global* g {garr(k, "g", 4U, {"1"})};
           Function* put {make_put(k, g, false)};
           Function* f {k.fn("f", k.vd)};
           const Expr* c {k.call(put, {})};
           f->body = k.blk({k.es(c)});
           k.done(f);
           k.fail_only("permission", c->loc, 7U, 0U);
       });

    // ---- item 7: overlap by slots, the aliasing trap ----
    // void two(int *p) requires(owns(*p) && owns(g)) ensures(owns(*p) && owns(g)) {}   int main() { two(&g[1]); }  (mode 1: two(&h), a distinct int: fine)
    auto alias_main = [=](Kit& k, bool alias)
    {
        Global* g {garr(k, "g", 4U, {"1", "2"})};
        Function* two {k.fn("two", k.vd, {{"p", PI(k)}})};
        k.req(two, k.land(k.b.atom(AtomKind::Owns, FP(k, two, 0), 0, k.at()), gatom(k, AtomKind::Owns, g, 1)));
        k.ens(two, k.land(k.b.atom(AtomKind::Owns, FP(k, two, 0), 0, k.at()), gatom(k, AtomKind::Owns, g, 1)));
        two->body = k.blk({});
        k.done(two);
        Function* m {k.fn("main", k.i32)};
        std::vector<const Stmt*> body;
        const Expr* c {nullptr};
        if (alias)
        {
            c = k.call(two, {gat(k, g, N(k, "1"))});
            body.push_back(k.es(c));
            k.fail_only("permission", c->loc, 18U, 8U);
        }
        else
        {
            Var* x {k.cell(m, "x", k.i32)};
            body.push_back(k.decl(x, N(k, "0")));
            body.push_back(k.es(k.call(two, {k.addr(x)})));
        }
        body.push_back(k.ret(N(k, "0")));
        m->body = k.blkc(body);
        k.done(m);
        k.prog.set_entry(m);
    };
    lc("garr_alias_trap",
       "two(&g[1]) with owns(*p) && owns(g): the callee would own g[1] twice: one permission error at the call",
       [=](Kit& k)
       {
           alias_main(k, true);
       });
    lc("garr_alias_distinct", "two(&x) of a distinct local: no overlap with g, no error",
       [=](Kit& k)
       {
           alias_main(k, false);
       });

    // ---- item 10: loops ----
    auto inc = [](Kit& k, Var* i)
    {
        return k.asg(i, k.add(k.v(i), k.num(k.i32, "1")));
    };
    // int g[3] = {7, 7, 7}; int main() { int *p = &g[1]; long s = 0; int i = 0; while (i < 3) inv 0 <= i <= 3 && s == 7 * i { s = s + g[i]; [*p = 1;] i++; } check(s == 21); return 0; }
    // mode 0 read-only (the values are kept), 1: the body also stores through p (reaches g: its values are not kept)
    auto sum_main = [=](Kit& k, int mode)
    {
        Global* g {garr(k, "g", 3U, {"7", "7", "7"})};
        Function* m {k.fn("main", k.i32)};
        Var* p {k.local(m, "p", PI(k))};
        Var* sv {k.local(m, "s", k.i64)};
        Var* i {k.local(m, "i", k.i32)};
        std::vector<const Stmt*> bs {k.asg(sv, k.add(k.v(sv), k.cv(k.i64, grd(k, g, k.pn(i)))))};
        if (mode == 1)
        {
            bs.push_back(k.store(k.pn(p), N(k, "1")));
        }
        bs.push_back(inc(k, i));
        const Expr* bound {k.range(k.v(i), k.i32, "0", "3")};
        const Clause kept {k.cl(k.eq(k.v(sv), k.mul(k.cv(k.i64, N(k, "7")), k.cv(k.i64, k.v(i)))))};
        const Stmt* c {k.chk(k.eq_c(k.v(sv), k.i64, "21"))};
        m->body =
            k.blkc({k.decl(p, gat(k, g, N(k, "1"))), k.decl(sv, k.num(k.i64, "0")), k.decl(i, N(k, "0")),
                    k.whl(k.lt(k.v(i), N(k, "3")), {k.cl(bound), kept}, {k.cl(k.sub(N(k, "3"), k.v(i)))}, k.blkc(bs)),
                    c, k.ret(N(k, "0"))});
        k.done(m);
        k.prog.set_entry(m);
        if (mode == 1)
        {
            k.fail("invariant-preserved", kept.loc);
        }
    };
    lc("garr_loop_sum",
       "a read-only loop over g[3] = {7, 7, 7} in main: the values are kept, s == 7 * i holds and s == 21 after the loop",
       [=](Kit& k)
       {
           sum_main(k, 0);
       });
    lc("garr_loop_pointer_write",
       "the body also stores through p = &g[1]: a store through a pointer reaches g, its values are not kept, s == 7 * i is not preserved",
       [=](Kit& k)
       {
           sum_main(k, 1);
       });
    // int g[4] = {1}; int main() { int i = 0; while (i < 4) inv 0 <= i <= 4 { g[i] = i; i++; } check(i == 4); check(g[2] == 0 [wrong: g[2] == 2 is not known]) }
    lc("garr_loop_fill",
       "a fill loop over g[4] in main: the loop writes the array (no values kept), the flags are kept, g[1] is readable after it",
       [=](Kit& k)
       {
           Global* g {garr(k, "g", 4U, {"1"})};
           Function* m {k.fn("main", k.i32)};
           Var* i {k.local(m, "i", k.i32)};
           const Stmt* body {k.blkc({gwr(k, g, k.pn(i), k.v(i)), inc(k, i)})};
           const Stmt* c {k.chk(k.eq_c(k.v(i), k.i32, "4"))};
           Var* x {k.local(m, "x", k.i32)};
           const Stmt* r {k.decl(x, grd(k, g, N(k, "1")))};
           m->body = k.blkc({k.decl(i, N(k, "0")),
                             k.whl(k.lt(k.v(i), N(k, "4")), {k.cl(k.range(k.v(i), k.i32, "0", "4"))},
                                   {k.cl(k.sub(N(k, "4"), k.v(i)))}, body),
                             c, r, k.ret(N(k, "0"))});
           k.done(m);
           k.prog.set_entry(m);
       });
    // a loop whose body reads g[i] for i <= 4: the index may be outside g (the bound row of the element read inside the loop)
    lc("garr_loop_read_outside",
       "while (i <= 4) { s = g[i]; ... } over int g[4]: out-of-bounds at the read in the body",
       [=](Kit& k)
       {
           Global* g {garr(k, "g", 4U, {"1"})};
           Function* m {k.fn("main", k.i32)};
           Var* i {k.local(m, "i", k.i32)};
           Var* sv {k.local(m, "s", k.i32)};
           const Expr* r {grd(k, g, k.pn(i))};
           const Stmt* body {k.blkc({k.asg(sv, r), inc(k, i)})};
           m->body = k.blkc({k.decl(sv, N(k, "0")), k.decl(i, N(k, "0")),
                             k.whl(k.le(k.v(i), N(k, "4")), {k.cl(k.range(k.v(i), k.i32, "0", "5"))},
                                   {k.cl(k.sub(N(k, "5"), k.v(i)))}, body),
                             k.ret(N(k, "0"))});
           k.done(m);
           k.prog.set_entry(m);
           k.fail("out-of-bounds", r->loc);
       });
    // void zero() requires(owns(g)) ensures(owns(g) && g[0] == 0) { int i = 0; while (i < 4) inv 0 <= i <= 4 { g[i] = 0; i++; } }  (a function that holds g through its atom: a range place)
    lc("garr_loop_callee",
       "a loop in a function that holds g through owns(g): a range place over the bound local, the loop writes the array",
       [=](Kit& k)
       {
           Global* g {garr(k, "g", 4U, {"1"})};
           Function* f {k.fn("zero", k.vd)};
           k.req(f, gatom(k, AtomKind::Owns, g, 0));
           k.ens(f, gatom(k, AtomKind::Owns, g, 0));
           Var* i {k.local(f, "i", k.i32)};
           const Stmt* body {k.blkc({gwr(k, g, k.pn(i), N(k, "0")), inc(k, i)})};
           f->body =
               k.blkc({k.decl(i, N(k, "0")), k.whl(k.lt(k.v(i), N(k, "4")), {k.cl(k.range(k.v(i), k.i32, "0", "4"))},
                                                   {k.cl(k.sub(N(k, "4"), k.v(i)))}, body)});
           k.done(f);
       });

    // int total() requires(reads(g)) ensures(reads(g)) { long s = 0; int i = 0; while (i < 3) inv 0 <= i <= 3 { s = s + g[i]; i++; } return 0; }  (a loop that READS g in a function holding it through reads(g))
    lc("garr_loop_reads_callee",
       "a read-only loop in a function that borrows g through reads(g): a range place at the atom's amount",
       [=](Kit& k)
       {
           Global* g {garr(k, "g", 3U, {"7", "7", "7"})};
           Function* f {k.fn("total", k.i32)};
           k.req(f, gatom(k, AtomKind::Reads, g, 0));
           k.ens(f, gatom(k, AtomKind::Reads, g, 0));
           Var* sv {k.local(f, "s", k.i32)};
           Var* i {k.local(f, "i", k.i32)};
           const Stmt* body {k.blkc({k.asg(sv, grd(k, g, k.pn(i))), inc(k, i)})};
           f->body = k.blkc({k.decl(sv, N(k, "0")), k.decl(i, N(k, "0")),
                             k.whl(k.lt(k.v(i), N(k, "3")), {k.cl(k.range(k.v(i), k.i32, "0", "3"))},
                                   {k.cl(k.sub(N(k, "3"), k.v(i)))}, body),
                             k.ret(N(k, "0"))});
           k.done(f);
       });
    // the same for a `static` local array of its own function (the implicit owns atom)
    lc("garr_loop_static_local",
       "int total() { static int g[3] = {7, 7, 7}; loop reading g[i] }: the implicit owns(g) is a range place",
       [=](Kit& k)
       {
           Function* f {k.fn("total", k.i32)};
           Global* g {garr(k, "g", 3U, {"7", "7", "7"}, false, f)};
           f->requires_.push_back(k.cl(gatom(k, AtomKind::Owns, g, 0, true)));
           f->ensures_.push_back(k.cl(gatom(k, AtomKind::Owns, g, 0, true)));
           Var* sv {k.local(f, "s", k.i32)};
           Var* i {k.local(f, "i", k.i32)};
           const Stmt* body {k.blkc({k.asg(sv, grd(k, g, k.pn(i))), inc(k, i)})};
           f->body = k.blkc({k.decl(sv, N(k, "0")), k.decl(i, N(k, "0")),
                             k.whl(k.lt(k.v(i), N(k, "3")), {k.cl(k.range(k.v(i), k.i32, "0", "3"))},
                                   {k.cl(k.sub(N(k, "3"), k.v(i)))}, body),
                             k.ret(N(k, "0"))});
           k.done(f);
       });

    // ---- item 7 (cut 7): a named global array as the argument of `forall` in a `check` ----
    auto mkq = [=](Kit& k, int id, const char* fname, const std::vector<const Expr*>& args,
                   const std::function<const Expr*(const Expr*, const std::vector<const Expr*>&)>& body)
    {
        const Expr* iv {k.b.bound(id, k.i32.int_type, "i", k.at())};
        std::vector<const Expr*> a;
        for (size_t t {0U}; t < args.size(); ++t)
        {
            const Type at {args[t]->type.is_ptr() && args[t]->type.pointee->is_array()
                               ? k.ptr(*args[t]->type.pointee->element)
                               : args[t]->type};
            a.push_back(
                k.b.forall_arg(id, static_cast<int>(t), at, args[t]->c_text.empty() ? "x" : args[t]->c_text, k.at()));
        }
        const Expr* b {body(iv, a)};
        return k.b.forall(id, k.i32.int_type, b, args, std::string("forall(") + fname + ")", fname, k.at());
    };
    // forall nonneg(g, n): for every i, !(0 <= i && i < n) || g[i] >= 0
    auto nonneg_g = [=](Kit& k, const Global* g, const char* n)
    {
        return mkq(k, 0, "nonneg", {ga(k, g), k.num(k.i32, n)},
                   [&](const Expr* iv, const std::vector<const Expr*>& x)
                   {
                       const Expr* in {k.land(k.ge_c(iv, k.i32, "0"), k.lt(iv, x[1]))};
                       return k.lor(k.lnot(in), k.ge_c(k.deref(k.b.elem_addr(x[0], iv, k.at())), k.i32, "0"));
                   });
    };
    // int g[4] = {1, 2, 3, 4}; int main() { check(forall nonneg(g, [4|5])); return 0; }  (mode 1: g[2] = -3, 2: five elements)
    auto forall_main = [=](Kit& k, int mode) -> const Stmt*
    {
        Global* g {garr(k, "g", 4U, {"1", "2", mode == 1 ? "-3" : "3", "4"})};
        Function* m {k.fn("main", k.i32)};
        const Stmt* c {k.chk(nonneg_g(k, g, mode == 2 ? "5" : "4"))};
        m->body = k.blk({c, k.ret(N(k, "0"))});
        k.done(m);
        k.prog.set_entry(m);
        return c;
    };
    lc("garr_forall_check",
       "check(forall nonneg(g, 4)) over a global array main holds: the bound row, the marked rows, the flags, then the fact",
       [=](Kit& k)
       {
           forall_main(k, 0);
       });
    lc("garr_forall_check_false_fact", "g[2] is -3: every row holds, the fact fails (check)",
       [=](Kit& k)
       {
           k.fail("check", forall_main(k, 1)->loc);
       });
    lc("garr_forall_check_outside",
       "the range is five elements of a four-element array: out-of-bounds at the quantifier's site",
       [=](Kit& k)
       {
           k.fail("out-of-bounds", forall_main(k, 2)->expr->loc, 1U, "", "outside `g`");
       });

    // ---- item 8: the decay of a global array passed to a range atom ----
    // void fill(int *p, int n) requires(owns_range(p, n)) ensures(owns_range(p, n)) {}   int main() { fill(g, [4|5]); int x = g[0]; return 0; }
    auto range_call_main = [=](Kit& k, const char* count) -> const Expr*
    {
        Global* g {garr(k, "g", 4U, {"1"})};
        Function* f {k.fn("fill", k.vd, {{"p", PI(k)}, {"n", k.i32}})};
        k.req(f, k.b.range_atom(AtomKind::OwnsRange, FP(k, f, 0), FP(k, f, 1), 0, k.at()));
        k.ens(f, k.b.range_atom(AtomKind::OwnsRange, FP(k, f, 0), FP(k, f, 1), 0, k.at()));
        f->body = k.blk({});
        k.done(f);
        Function* m {k.fn("main", k.i32)};
        Var* x {k.local(m, "x", k.i32)};
        const Expr* c {k.call(f, {k.b.text(gat(k, g, N(k, "0")), "g"), N(k, count)})};
        m->body = k.blk({k.es(c), k.decl(x, grd(k, g, N(k, "0"))), k.ret(N(k, "0"))});
        k.done(m);
        k.prog.set_entry(m);
        return c;
    };
    lc("garr_range_call",
       "fill(g, 4): the decay of g bound to a local, e1b's rows over it (the whole array), g[0] readable after",
       [=](Kit& k)
       {
           range_call_main(k, "4");
       });
    lc("garr_range_call_too_long",
       "fill(g, 5) over a four-element array: the shape row of the range (the range is longer than its object) fails",
       [=](Kit& k)
       {
           k.fail_only("permission", range_call_main(k, "5")->loc, 8U, 0U);
       });

    // ---- item 9 (a `static` local array): its implicit atom ----
    // int counter() { static int g[2] = {0, 0}; g[0] = 5; return g[0]; }   int main() { check(counter() == 5); return 0; }
    lc("garr_static_local",
       "a static local array's implicit owns atom: counter() takes and returns it; main holds it from program_start and calls it",
       [=](Kit& k)
       {
           Function* f {k.fn("counter", k.i32)};
           Global* g {garr(k, "g", 2U, {"0", "0"}, false, f)};
           f->requires_.push_back(k.cl(gatom(k, AtomKind::Owns, g, 0, true)));
           f->ensures_.push_back(k.cl(gatom(k, AtomKind::Owns, g, 0, true)));
           k.ens(f, k.eq_c(k.R(f), k.i32, "5"));
           const Stmt* w {gwr(k, g, N(k, "0"), N(k, "5"))};
           f->body = k.blk({w, k.ret(grd(k, g, N(k, "0")))});
           k.done(f);
           Function* m {k.fn("main", k.i32)};
           Var* r {k.local(m, "r", k.i32)};
           const Stmt* d {k.decl(r, k.call(f, {}))};
           const Stmt* c {k.chk(k.eq_c(k.v(r), k.i32, "5"))};
           m->body = k.blk({d, c, k.ret(N(k, "0"))});
           k.done(m);
           k.prog.set_entry(m);
       });

    // void two(int *p) requires(owns(*p) && owns(g)) ensures(owns(*p) && owns(g)) { *p = 1; g[1] = 2; check(*p == 1 && g[1] == 2); }
    // the body must know that the cell and the array do not overlap (item 7: the facts of `f__wf` by slot, and Silicon's own reasoning about the two chunks)
    auto two_body = [=](Kit& k, bool wrong)
    {
        Global* g {garr(k, "g", 4U, {"1", "2"})};
        Function* two {k.fn("two", k.vd, {{"p", PI(k)}})};
        k.req(two, k.land(k.b.atom(AtomKind::Owns, FP(k, two, 0), 0, k.at()), gatom(k, AtomKind::Owns, g, 1)));
        k.ens(two, k.land(k.b.atom(AtomKind::Owns, FP(k, two, 0), 0, k.at()), gatom(k, AtomKind::Owns, g, 1)));
        const Stmt* w1 {k.store(k.pn(two->param_var(0)), N(k, "1"))};
        const Stmt* w2 {gwr(k, g, N(k, "1"), N(k, "2"))};
        const Stmt* c {k.chk(k.land(k.eq_c(k.deref(k.pn(two->param_var(0))), k.i32, wrong ? "2" : "1"),
                                    k.eq_c(grd(k, g, N(k, "1")), k.i32, "2")))};
        two->body = k.blk({w1, w2, c});
        k.done(two);
        if (wrong)
        {
            k.fail("check", c->loc);
        }
    };
    lc("garr_callee_cell_and_array",
       "a callee owning *p and g writes both and reads them back: the cell and the array are distinct objects",
       [=](Kit& k)
       {
           two_body(k, false);
       });
    lc("garr_callee_cell_and_array_wrong",
       "the same claiming *p == 2: the check fails (the facts are not inconsistent)",
       [=](Kit& k)
       {
           two_body(k, true);
       });

    // int g[3] = {1, 2, 3}; int *gp = g; int main() { check(gp == &g[0]); check(gp[0] == 1); return 0; }  (item 9: the decay as an initial value is the address of element 0)
    lc("garr_addr_target",
       "a pointer global initialized with the decay of an array global holds loc(GB_g(), 0): gp[1]'s element is g[1]",
       [=](Kit& k)
       {
           Global* g {garr(k, "g", 3U, {"1", "2", "3"})};
           Global* gp {k.prog.add_global("gp", PI(k), k.at(), nullptr)};
           gp->init_kind = GlobalInit::Addr;
           gp->target = g;
           Function* m {k.fn("main", k.i32)};
           const Stmt* c {k.chk(k.eq(k.deref(ga(k, gp)), gat(k, g, N(k, "0"))))};
           m->body = k.blk({c, k.ret(N(k, "0"))});
           k.done(m);
           k.prog.set_entry(m);
       });

    // ---- item 7 (cut 7): a `pure` function applied to a named global array in a `check` ----
    // long sum_to(const int *a, int n) requires(0 <= n && reads_range(a, n)) ensures(r, -2147483648LL * n <= r && r <= 2147483647LL * n) decreases(n)
    //     { return n == 0 ? 0 : sum_to(a, n - 1) + a[n - 1]; }
    auto mk_sum_g = [=](Kit& k) -> Function*
    {
        Function* f {k.fn("sum_to", k.i64, {{"a", PI(k)}, {"n", k.i32}})};
        f->pure = true;
        k.req(f, k.land(k.ge_c(FP(k, f, 1), k.i32, "0"),
                        k.b.range_atom(AtomKind::ReadsRange, FP(k, f, 0), FP(k, f, 1), 0, k.at())));
        const Expr* lo {k.mul(k.num(k.i64, "-2147483648"), k.cv(k.i64, FP(k, f, 1)))};
        const Expr* hi {k.mul(k.num(k.i64, "2147483647"), k.cv(k.i64, FP(k, f, 1)))};
        k.ens(f, k.land(k.le(lo, k.R(f)), k.le(k.R(f), hi)));
        k.dec(f, FP(k, f, 1));
        const Expr* nm1 {k.sub(k.F(f, 1), k.num(k.i32, "1"))};
        const Expr* self {k.b.apply(f, {k.F(f, 0), nm1}, "sum_to(a, n - 1)", k.at())};
        const Expr* elem {k.deref(k.b.elem_addr(k.F(f, 0), k.sub(k.F(f, 1), k.num(k.i32, "1")), k.at()))};
        f->pure_body = k.cond(k.eq(k.F(f, 1), k.num(k.i32, "0")), k.num(k.i64, "0"), k.add(self, k.cv(k.i64, elem)));
        k.done(f);
        return f;
    };
    // int g[3] = {1, 2, 3}; int main() { check(sum_to(g, <count>) <= 6442450941 | < 0); return 0; }   fact 0 holds, 1 does not; count 4 is past the array
    auto pure_main = [=](Kit& k, const char* count, int fact, const Expr** app_out) -> const Stmt*
    {
        Global* g {garr(k, "g", 3U, {"1", "2", "3"})};
        Function* sum {mk_sum_g(k)};
        Function* m {k.fn("main", k.i32)};
        const Expr* ap {k.b.apply(sum, {ga(k, g), N(k, count)}, "sum_to(g, n)", k.at())};
        *app_out = ap;
        const Stmt* c {k.chk(fact == 0 ? k.le(ap, k.num(k.i64, "6442450941")) : k.lt(ap, k.num(k.i64, "0")))};
        m->body = k.blk({c, k.ret(N(k, "0"))});
        k.done(m);
        k.prog.set_entry(m);
        return c;
    };
    lc("garr_pure_check",
       "check(sum_to(g, 3) <= ...) over a global array main holds: the rows over the bound local, then the fact",
       [=](Kit& k)
       {
           const Expr* a {nullptr};
           pure_main(k, "3", 0, &a);
       });
    lc("garr_pure_check_false_fact", "check(sum_to(g, 3) < 0): the rows hold, the fact is not provable (check)",
       [=](Kit& k)
       {
           const Expr* a {nullptr};
           k.fail("check", pure_main(k, "3", 1, &a)->loc);
       });
    lc("garr_pure_check_past",
       "sum_to(g, 4) over a three-element array: the callee's range line is not given (precondition at the application)",
       [=](Kit& k)
       {
           const Expr* a {nullptr};
           pure_main(k, "4", 0, &a);
           k.fail_only("precondition", a->loc, 5U, 3U);
       });

    // ---- item 10 with item 7: a `forall` and a pure application over a named global array in a loop invariant ----
    // int main() { int i = 0; while (i < 3) inv 0 <= i <= 3 && [forall nonneg(g, 3) | sum_to(g, 3) <= 6442450941] { i++; } return 0; }
    auto inv_main = [=](Kit& k, bool with_forall, bool bad_count) -> const Stmt*
    {
        Global* g {garr(k, "g", 3U, {"1", "2", "3"})};
        Function* sum {with_forall ? nullptr : mk_sum_g(k)};
        Function* m {k.fn("main", k.i32)};
        Var* i {k.local(m, "i", k.i32)};
        const Expr* fact {with_forall
                              ? nonneg_g(k, g, bad_count ? "4" : "3")
                              : k.le(k.b.apply(sum, {ga(k, g), N(k, bad_count ? "4" : "3")}, "sum_to(g, n)", k.at()),
                                     k.num(k.i64, "6442450941"))};
        const Stmt* lp {k.whl(k.lt(k.v(i), N(k, "3")), {k.cl(k.range(k.v(i), k.i32, "0", "3")), k.cl(fact)},
                              {k.cl(k.sub(N(k, "3"), k.v(i)))}, k.blkc({inc(k, i)}))};
        m->body = k.blkc({k.decl(i, N(k, "0")), lp, k.ret(N(k, "0"))});
        k.done(m);
        k.prog.set_entry(m);
        return lp;
    };
    lc("garr_forall_invariant",
       "forall nonneg(g, 3) in the invariant of a loop in main (the array is a place of the loop, the values are kept): verifies",
       [=](Kit& k)
       {
           inv_main(k, true, false);
       });
    lc("garr_pure_invariant",
       "sum_to(g, 3) <= ... in a loop invariant over a global array: the rows, then the fact, verify",
       [=](Kit& k)
       {
           inv_main(k, false, false);
       });
}

// Array compound literals in C (decision 0063 item 12, part D's lowering half): `(int[]){1, 2, 3}` is a hidden array local of the enclosing block, declared before its statement (allocated at the
// block's entry, zero-filled at the declaration for a list that leaves elements out), written inside the expression by element stores in order, its value the decay (the address of element 0).
void define_array_literals()
{
    auto N = [](Kit& k, const char* v)
    {
        return k.num(k.i32, v);
    };
    // the hidden object `(int [n]){...}`: an array local, a cell, named by the literal's spelling in messages
    auto lit = [](Kit& k, Function* f, unsigned long long n, bool zero_filled) -> Var*
    {
        Var* x {f->new_local("(int [" + std::to_string(n) + "]){...}", k.prog.array_of(k.i32, n), k.at())};
        f->mark_address_taken(x);
        x->compound_literal = true;
        x->zero_filled = zero_filled;
        return x;
    };
    auto el = [](Kit& k, Var* x, const Expr* i)
    {
        return k.b.elem_addr(k.addr(x), i, k.at());
    };
    // the expression `(lit[0] = v0, lit[1] = v1, ..., &lit[0])`: the stores in order, the decay last
    auto expr_of = [=](Kit& k, Var* x, const std::vector<const char*>& vals) -> const Expr*
    {
        const Expr* chain {nullptr};
        for (size_t i {0U}; i < vals.size(); ++i)
        {
            const Expr* st {k.b.store_expr(el(k, x, N(k, std::to_string(i).c_str())), N(k, vals[i]), k.at())};
            chain = chain == nullptr ? st : k.comma(chain, st);
        }
        return chain == nullptr ? el(k, x, N(k, "0")) : k.comma(chain, el(k, x, N(k, "0")));
    };
    // int main() { int *p = (int[]){1, 2, 3}; check(p[1] == [2|3]); return 0; }   (p[i] reads through the pointer p)
    auto lit_main = [=](Kit& k, unsigned long long n, const std::vector<const char*>& vals, const char* index,
                        const char* want, bool zero) -> const Stmt*
    {
        Function* m {k.fn("main", k.i32)};
        Var* x {lit(k, m, n, zero)};
        Var* p {k.local(m, "p", k.ptr(k.i32))};
        const Expr* r {k.deref(k.b.elem_addr(k.b.text(k.v(p), "p"), N(k, index), k.at()))};
        const Stmt* c {k.chk(k.eq_c(r, k.i32, want))};
        m->body = k.blkc({k.decl(x, nullptr), k.decl(p, expr_of(k, x, vals)), c, k.ret(N(k, "0"))});
        k.done(m);
        return c;
    };
    lc("alit_main",
       "(int[]){1, 2, 3}: a hidden array local, three element stores in the expression, the decay in p; p[1] == 2",
       [=](Kit& k)
       {
           lit_main(k, 3U, {"1", "2", "3"}, "1", "2", false);
       });
    lc("alit_main_wrong", "the same, p[1] == 3 fails (check)",
       [=](Kit& k)
       {
           k.fail("check", lit_main(k, 3U, {"1", "2", "3"}, "1", "3", false)->loc);
       });
    lc("alit_zero_rest",
       "(int[4]){1}: the list leaves three elements out: zero_arr at the declaration, then the store of element 0; p[2] == 0",
       [=](Kit& k)
       {
           lit_main(k, 4U, {"1"}, "2", "0", true);
       });
    lc("alit_outside", "p[3] of a three-element literal: the bound row of the element read through p",
       [=](Kit& k)
       {
           Function* m {k.fn("main", k.i32)};
           Var* x {lit(k, m, 3U, false)};
           Var* p {k.local(m, "p", k.ptr(k.i32))};
           const Expr* r {k.deref(k.b.elem_addr(k.b.text(k.v(p), "p"), N(k, "3"), k.at()))};
           m->body = k.blkc({k.decl(x, nullptr), k.decl(p, expr_of(k, x, {"1", "2", "3"})),
                             k.chk(k.eq_c(r, k.i32, "0")), k.ret(N(k, "0"))});
           k.done(m);
           k.fail("out-of-bounds", r->loc);
       });
    lc("alit_unwritten",
       "a literal list that writes fewer elements than the array and is not zero-filled: the element read has no value (uninitialized)",
       [=](Kit& k)
       {
           Function* m {k.fn("main", k.i32)};
           Var* x {lit(k, m, 3U, false)};
           Var* p {k.local(m, "p", k.ptr(k.i32))};
           const Expr* r {k.deref(k.b.elem_addr(k.b.text(k.v(p), "p"), N(k, "2"), k.at()))};
           m->body = k.blkc(
               {k.decl(x, nullptr), k.decl(p, expr_of(k, x, {"1"})), k.chk(k.eq_c(r, k.i32, "0")), k.ret(N(k, "0"))});
           k.done(m);
           k.fail("uninitialized", r->loc);
       });
}

// ---- array members of structures (decision 0063 part C: items 2, 3, 5-11 for `struct S { int n; int a[4]; }`) ----
void define_member_arrays()
{
    auto N = [](Kit& k, const char* v)
    {
        return k.num(k.i32, v);
    };
    // struct S { int n; int a[4]; }: member 0 is the cell n, member 1 the array leaf a
    auto SA = [](Kit& k)
    {
        return k.strukt("S", {{"n", k.i32}, {"a", k.prog.array_of(k.i32, 4U)}});
    };
    // `base->a[i]` / `s.a[i]`: the address, a read, a store (`base` is `&s` or the pointer `p`)
    auto ea = [=](Kit& k, const Expr* base, const char* i)
    {
        return k.b.elem_addr(k.mem(base, 1), N(k, i), k.at());
    };
    auto rd = [=](Kit& k, const Expr* base, const char* i)
    {
        return k.deref(ea(k, base, i));
    };
    auto wr = [=](Kit& k, const Expr* base, const char* i, const char* v)
    {
        return k.store(ea(k, base, i), N(k, v));
    };
    auto rdn = [=](Kit& k, const Expr* base)
    {
        return k.leaf(base, 0);
    };
    // a positive case and its vacuity twin (`check(false)` at the end, which must fail: the allocation's rows are consistent)
    auto lcv = [](const char* name, const char* doc, const std::function<void(Kit&)>& build)
    {
        lc(name, doc, build);
        lc(std::string(name) + "_vacuity",
           "the same with check(false) after everything: it fails (the rows are consistent)",
           [build](Kit& k)
           {
               k.vac_tail = true;
               build(k);
               k.fail("check", k.vac_loc);
           });
    };
    auto tail = [](Kit& k, std::vector<const Stmt*>& body)
    {
        if (k.vac_tail)
        {
            const Stmt* v {k.chk(k.no())};
            body.push_back(v);
            k.vac_loc = v->loc;
        }
    };

    // void f(void) { struct S s; s.a[1] = 5; s.n = 7; check(s.a[1] == 5 && s.n == 7); }
    lcv("marr_local_rw",
        "a structure local with an array member: an element written and read beside the other member, then the scope exit",
        [=](Kit& k)
        {
            StructType* S {SA(k)};
            Function* f {k.fn("f", k.vd)};
            Var* s {k.scell(f, "s", S)};
            std::vector<const Stmt*> body {
                k.decl(s, nullptr), wr(k, k.addr(s), "1", "5"), k.lstore(k.addr(s), 0, N(k, "7")),
                k.chk(k.land(k.eq_c(rd(k, k.addr(s), "1"), k.i32, "5"), k.eq_c(rdn(k, k.addr(s)), k.i32, "7")))};
            tail(k, body);
            f->body = k.blkc(body);
            k.done(f);
        });
    lc("marr_local_index_outside", "s.a[4] of int a[4]: the bound row (out-of-bounds) at the read",
       [=](Kit& k)
       {
           StructType* S {SA(k)};
           Function* f {k.fn("f", k.vd)};
           Var* s {k.scell(f, "s", S)};
           const Expr* r {rd(k, k.addr(s), "4")};
           f->body = k.blkc({k.decl(s, nullptr), k.chk(k.eq_c(r, k.i32, "0"))});
           k.done(f);
           k.fail("out-of-bounds", r->loc);
       });
    lc("marr_local_unwritten", "s.a[2] read before any write: uninitialized",
       [=](Kit& k)
       {
           StructType* S {SA(k)};
           Function* f {k.fn("f", k.vd)};
           Var* s {k.scell(f, "s", S)};
           const Expr* r {rd(k, k.addr(s), "2")};
           f->body = k.blkc({k.decl(s, nullptr), wr(k, k.addr(s), "1", "5"), k.chk(k.eq_c(r, k.i32, "0"))});
           k.done(f);
           k.fail("uninitialized", r->loc);
       });
    // struct S s = {7, {1, 2}}: the zero fill of the member, then the listed elements, then the other member
    auto init_local = [=](Kit& k, const char* wrong)
    {
        StructType* S {SA(k)};
        Function* f {k.fn("f", k.vd)};
        Var* s {k.scell(f, "s", S)};
        const Expr* last {rd(k, k.addr(s), "2")};
        const Stmt* c {k.chk(k.land(k.eq_c(last, k.i32, wrong), k.land(k.eq_c(rd(k, k.addr(s), "1"), k.i32, "2"),
                                                                       k.eq_c(rdn(k, k.addr(s)), k.i32, "7"))))};
        f->body = k.blkc({k.decl(s, nullptr), k.b.array_zero(k.mem(k.addr(s), 1), k.at()), wr(k, k.addr(s), "0", "1"),
                          wr(k, k.addr(s), "1", "2"), k.lstore(k.addr(s), 0, N(k, "7")), c});
        k.done(f);
        if (std::string(wrong) != "0")
        {
            k.fail("check", c->loc);
        }
    };
    // ---- owns(*p), reads(*p) and owns(p->a) at a call and in the callee (items 7 and 8) ----
    auto FPs = [](Kit& k, Function* f, int i)
    {
        return k.b.text(k.F(f, i), f->params[static_cast<size_t>(i)].name);
    };
    auto PS = [](Kit& k, StructType* S)
    {
        return k.ptr(k.st(S));
    };
    // `s` with every element and the cell written: struct S s; s.n = 7; s.a = {1, 2, 3, 4}
    auto fill = [=](Kit& k, Var* s, std::vector<const Stmt*>& body, const char* n)
    {
        body.push_back(k.decl(s, nullptr));
        body.push_back(k.lstore(k.addr(s), 0, N(k, n)));
        for (const char* i : {"0", "1", "2", "3"})
        {
            body.push_back(
                wr(k, k.addr(s), i,
                   std::string(i) == "0" ? "1" : (std::string(i) == "1" ? "2" : (std::string(i) == "2" ? "3" : "4"))));
        }
    };
    // void touch(struct S *p, int i) requires(owns(*p) && 0 <= i && i < 4) ensures(owns(*p) && p->n == 3 && p->a[i] == old(p->n)) { p->a[i] = p->n; p->n = 3; }
    auto make_touch = [=](Kit& k, StructType* S, bool wrong) -> Function*
    {
        Function* f {k.fn("touch", k.vd, {{"p", PS(k, S)}, {"i", k.i32}})};
        k.req(f, k.land(k.b.atom(AtomKind::Owns, FPs(k, f, 0), 0, k.at()),
                        k.land(k.le(N(k, "0"), k.F(f, 1)), k.lt(k.F(f, 1), N(k, "4")))));
        const Expr* elem_c {k.deref(k.b.elem_addr(k.mem(FPs(k, f, 0), 1), k.F(f, 1), k.at()))};
        k.ens(f, k.land(k.b.atom(AtomKind::Owns, FPs(k, f, 0), 0, k.at()),
                        k.land(k.eq_c(k.leaf(FPs(k, f, 0), 0), k.i32, "3"),
                               k.eq(elem_c, k.old(k.leaf(FPs(k, f, 0), 0))))));
        const Stmt* w1 {k.store(k.b.elem_addr(k.mem(k.pv(f, 0), 1), k.pv(f, 1), k.at()), k.leaf(k.pv(f, 0), 0))};
        const Stmt* w2 {k.lstore(k.pv(f, 0), 0, N(k, wrong ? "4" : "3"))};
        f->body = k.blk({w1, w2});
        k.done(f);
        if (wrong)
        {
            k.fail("postcondition", f->end_loc);
        }
        return f;
    };
    // struct S s = {7, {1, 2, 3, 4}}; touch(&s, 2); check(s.a[2] == 7 && s.n == 3 && s.a[0] == 1);
    auto touch_main = [=](Kit& k, bool wrong_check)
    {
        StructType* S {SA(k)};
        Function* touch {make_touch(k, S, false)};
        Function* f {k.fn("use", k.vd)};
        Var* s {k.scell(f, "s", S)};
        std::vector<const Stmt*> body;
        fill(k, s, body, "7");
        body.push_back(k.es(k.call(touch, {k.addr(s), N(k, "2")})));
        // the callee owns the whole of s and promises only s.n == 3 and s.a[2] == old(s.n): s.a[0] == 1 is not known after the call
        const Stmt* c {wrong_check ? k.chk(k.eq_c(rd(k, k.addr(s), "0"), k.i32, "1"))
                                   : k.chk(k.land(k.eq_c(rd(k, k.addr(s), "2"), k.i32, "7"),
                                                  k.eq_c(rdn(k, k.addr(s)), k.i32, "3")))};
        body.push_back(c);
        tail(k, body);
        f->body = k.blkc(body);
        k.done(f);
        if (wrong_check)
        {
            k.fail("check", c->loc);
        }
    };
    lcv("marr_owns_call",
        "touch(&s, 2) takes owns(*p): the cell leaf's rows and the array leaf's marked rows over a bound local; the callee writes p->a[i] and p->n; the caller reads them back",
        [=](Kit& k)
        {
            touch_main(k, false);
        });
    // struct T { int x; struct S s; } t; touch(&t.s, 1): the argument is the address of a nested member, bound to a local before the parts' own
    lcv("marr_nested_call",
        "touch(&t.s, 1) on a nested structure: the argument is bound to a local first, the callee's array leaf is t.s.a",
        [=](Kit& k)
        {
            StructType* S {SA(k)};
            Function* touch {make_touch(k, S, false)};
            StructType* T {k.strukt("T", {{"x", k.i32}, {"s", k.st(S)}})};
            Function* f {k.fn("use", k.vd)};
            Var* t {k.scell(f, "t", T)};
            const Expr* ts {k.mem(k.addr(t), 1)};
            auto ta = [&](const char* i)
            {
                return k.b.elem_addr(k.mem(ts, 1), N(k, i), k.at());
            };
            std::vector<const Stmt*> body {k.decl(t, nullptr), k.lstore(k.addr(t), 0, N(k, "1")),
                                           k.lstore(ts, 0, N(k, "7"))};
            for (const char* i : {"0", "1", "2", "3"})
            {
                body.push_back(k.store(ta(i), N(k, "2")));
            }
            body.push_back(k.es(k.call(touch, {ts, N(k, "1")})));
            body.push_back(k.chk(k.land(k.eq_c(k.deref(ta("1")), k.i32, "7"), k.eq_c(k.leaf(ts, 0), k.i32, "3"))));
            tail(k, body);
            f->body = k.blkc(body);
            k.done(f);
        });
    lc("marr_owns_call_forgot",
       "touch promises only p->n == 3 and p->a[i] == old(p->n): s.a[0] == 1 after the call is not known, the check fails",
       [=](Kit& k)
       {
           touch_main(k, true);
       });
    lc("marr_owns_callee_wrong", "touch writes 4 where it promises 3: the postcondition fails at the closing brace",
       [=](Kit& k)
       {
           make_touch(k, SA(k), true);
       });
    // int peek(struct S *p) requires(reads(*p)) ensures(reads(*p) && result == p->a[2]) { return p->a[2]; }  (mode 1: p->a[1] = 0 under reads: permission)
    auto make_peek = [=](Kit& k, StructType* S, bool writes) -> Function*
    {
        Function* f {k.fn("peek", k.i32, {{"p", PS(k, S)}})};
        k.req(f, k.b.atom(AtomKind::Reads, FPs(k, f, 0), 0, k.at()));
        const Expr* elem_c {k.deref(k.b.elem_addr(k.mem(FPs(k, f, 0), 1), N(k, "2"), k.at()))};
        k.ens(f, k.land(k.b.atom(AtomKind::Reads, FPs(k, f, 0), 0, k.at()), k.eq(k.R(f), elem_c)));
        std::vector<const Stmt*> body;
        const Stmt* w {nullptr};
        if (writes)
        {
            w = k.store(k.b.elem_addr(k.mem(k.pv(f, 0), 1), N(k, "1"), k.at()), N(k, "0"));
            body.push_back(w);
        }
        body.push_back(k.ret(k.deref(k.b.elem_addr(k.mem(k.pv(f, 0), 1), N(k, "2"), k.at()))));
        f->body = k.blk(body);
        k.done(f);
        if (writes)
        {
            k.fail("permission", w->loc);
        }
        return f;
    };
    lcv("marr_reads_call",
        "peek(&s) takes reads(*p): the array leaf at a fraction, the callee reads p->a[2]; the caller still owns s",
        [=](Kit& k)
        {
            StructType* S {SA(k)};
            Function* peek {make_peek(k, S, false)};
            Function* f {k.fn("use", k.vd)};
            Var* s {k.scell(f, "s", S)};
            Var* r {k.local(f, "r", k.i32)};
            std::vector<const Stmt*> body;
            fill(k, s, body, "7");
            body.push_back(k.decl(r, k.call(peek, {k.addr(s)})));
            body.push_back(k.chk(k.eq_c(k.v(r), k.i32, "3")));
            body.push_back(wr(k, k.addr(s), "1", "6"));
            body.push_back(k.chk(k.eq_c(rd(k, k.addr(s), "1"), k.i32, "6")));
            tail(k, body);
            f->body = k.blkc(body);
            k.done(f);
        });
    lc("marr_reads_write", "an element written under reads(*p): permission at the store",
       [=](Kit& k)
       {
           make_peek(k, SA(k), true);
       });
    // void put(struct S *p) requires(owns(p->a)) ensures(owns(p->a) && p->a[1] == 5) { p->a[1] = 5; [p->n = 1;] }
    auto make_put = [=](Kit& k, StructType* S, bool writes_n) -> Function*
    {
        Function* f {k.fn("put", k.vd, {{"p", PS(k, S)}})};
        k.req(f, k.b.atom(AtomKind::Owns, k.mem(FPs(k, f, 0), 1), 0, k.at()));
        const Expr* elem_c {k.deref(k.b.elem_addr(k.mem(FPs(k, f, 0), 1), N(k, "1"), k.at()))};
        k.ens(f, k.land(k.b.atom(AtomKind::Owns, k.mem(FPs(k, f, 0), 1), 0, k.at()), k.eq_c(elem_c, k.i32, "5")));
        std::vector<const Stmt*> body {wr(k, k.pv(f, 0), "1", "5")};
        const Stmt* w {nullptr};
        if (writes_n)
        {
            w = k.lstore(k.pv(f, 0), 0, N(k, "1"));
            body.push_back(w);
        }
        f->body = k.blk(body);
        k.done(f);
        if (writes_n)
        {
            k.fail("permission", w->loc);
        }
        return f;
    };
    lcv("marr_owns_member",
        "owns(p->a) alone: the whole array member and nothing else; the callee writes p->a[1]; the caller keeps its cell",
        [=](Kit& k)
        {
            StructType* S {SA(k)};
            Function* put {make_put(k, S, false)};
            Function* f {k.fn("use", k.vd)};
            Var* s {k.scell(f, "s", S)};
            std::vector<const Stmt*> body;
            fill(k, s, body, "7");
            body.push_back(k.es(k.call(put, {k.addr(s)})));
            body.push_back(
                k.chk(k.land(k.eq_c(rd(k, k.addr(s), "1"), k.i32, "5"), k.eq_c(rdn(k, k.addr(s)), k.i32, "7"))));
            tail(k, body);
            f->body = k.blkc(body);
            k.done(f);
        });
    lc("marr_owns_member_writes_cell", "p->n written under owns(p->a): permission at the store",
       [=](Kit& k)
       {
           make_put(k, SA(k), true);
       });
    // ---- the alias trap (item 7) ----
    // void two(struct S *p, int *q) requires(owns(*p) && owns(*q)) ensures(owns(*p) && owns(*q)) {}
    auto make_two = [=](Kit& k, StructType* S) -> Function*
    {
        Function* f {k.fn("two", k.vd, {{"p", PS(k, S)}, {"q", k.ptr(k.i32)}})};
        k.req(f, k.land(k.b.atom(AtomKind::Owns, FPs(k, f, 0), 0, k.at()),
                        k.b.atom(AtomKind::Owns, FPs(k, f, 1), 1, k.at())));
        k.ens(f, k.land(k.b.atom(AtomKind::Owns, FPs(k, f, 0), 0, k.at()),
                        k.b.atom(AtomKind::Owns, FPs(k, f, 1), 1, k.at())));
        f->body = k.blk({});
        k.done(f);
        return f;
    };
    // two(&s, &o) verifies; two(&s, &s.a[1]) aliases an element of s with *q: one permission error (the separation row)
    auto alias_main = [=](Kit& k, bool alias)
    {
        StructType* S {SA(k)};
        Function* two {make_two(k, S)};
        Function* f {k.fn("use", k.vd)};
        Var* s {k.scell(f, "s", S)};
        Var* o {k.cell(f, "o", k.i32)};
        std::vector<const Stmt*> body;
        fill(k, s, body, "7");
        body.push_back(k.decl(o, N(k, "0")));
        const Expr* q {alias ? k.b.elem_addr(k.mem(k.addr(s), 1), N(k, "1"), k.at()) : k.addr(o)};
        const Expr* call {k.call(two, {k.addr(s), q})};
        body.push_back(k.es(call));
        tail(k, body);
        f->body = k.blkc(body);
        k.done(f);
        if (alias)
        {
            // the 25 checks of the call (its rows): only the 15th, the cell q against the range of s.a, fails; the rest hold after it
            k.fail_only("permission", call->loc, 25U, 15U);
        }
    };
    lcv("marr_alias_distinct", "two(&s, &o): the cell o is outside s.a's range: the separation row holds",
        [=](Kit& k)
        {
            alias_main(k, false);
        });
    lc("marr_alias_trap",
       "two(&s, &s.a[1]) with owns(*p) && owns(*q): the cell q lies in s.a: one permission error at the call",
       [=](Kit& k)
       {
           alias_main(k, true);
       });
    // audit round 3, S2: void two_r(struct S *p, int *q) requires(reads(*p) && reads(*q)) ensures(reads(*p) && reads(*q)) { [*q = 1;] }   two_r(&s, &s.a[1]): two read sites share a slot, which is allowed;
    // the array part's marked rows name the other site's amount, bound to a Perm local (never `perm` under the quantifier)
    auto reads_alias = [=](Kit& k, bool writes)
    {
        StructType* S {SA(k)};
        Function* f {k.fn("two_r", k.vd, {{"p", PS(k, S)}, {"q", k.ptr(k.i32)}})};
        k.req(f, k.land(k.b.atom(AtomKind::Reads, FPs(k, f, 0), 0, k.at()),
                        k.b.atom(AtomKind::Reads, FPs(k, f, 1), 1, k.at())));
        k.ens(f, k.land(k.b.atom(AtomKind::Reads, FPs(k, f, 0), 0, k.at()),
                        k.b.atom(AtomKind::Reads, FPs(k, f, 1), 1, k.at())));
        const Stmt* st {writes ? k.store(k.pv(f, 1), N(k, "1")) : nullptr};
        f->body = writes ? k.blk({st}) : k.blk({});
        k.done(f);
        if (writes)
        {
            k.fail("permission", st->loc);
            return;
        }
        Function* u {k.fn("use_r", k.vd)};
        Var* s {k.scell(u, "s", S)};
        std::vector<const Stmt*> body;
        fill(k, s, body, "7");
        body.push_back(k.es(k.call(f, {k.addr(s), k.b.elem_addr(k.mem(k.addr(s), 1), N(k, "1"), k.at())})));
        tail(k, body);
        u->body = k.blkc(body);
        k.done(u);
    };
    lcv("marr_alias_reads",
        "two_r(&s, &s.a[1]) with reads(*p) && reads(*q): the cell q lies in s.a, two readers may share a slot; the array part's rows add q's amount as a Perm local",
        [=](Kit& k)
        {
            reads_alias(k, false);
        });
    lc("marr_alias_reads_write", "the callee stores through q under reads(*q): permission at the store",
       [=](Kit& k)
       {
           reads_alias(k, true);
       });
    // ---- two array leaves (audit round 1, S3): struct W { int a[4]; }, void two_w(struct W *p, struct W *q) requires(owns(*p) && owns(*q)) ----
    auto make_two_w = [=](Kit& k, StructType* W) -> Function*
    {
        Function* f {k.fn("two_w", k.vd, {{"p", PS(k, W)}, {"q", PS(k, W)}})};
        k.req(f, k.land(k.b.atom(AtomKind::Owns, FPs(k, f, 0), 0, k.at()),
                        k.b.atom(AtomKind::Owns, FPs(k, f, 1), 1, k.at())));
        k.ens(f, k.land(k.b.atom(AtomKind::Owns, FPs(k, f, 0), 0, k.at()),
                        k.b.atom(AtomKind::Owns, FPs(k, f, 1), 1, k.at())));
        f->body = k.blk({});
        k.done(f);
        return f;
    };
    auto two_w_main = [=](Kit& k, bool alias)
    {
        StructType* W {k.strukt("W", {{"a", k.prog.array_of(k.i32, 4U)}})};
        Function* two_w {make_two_w(k, W)};
        Function* f {k.fn("use", k.vd)};
        Var* w1 {k.scell(f, "w1", W)};
        Var* w2 {k.scell(f, "w2", W)};
        std::vector<const Stmt*> body {k.decl(w1, nullptr), k.decl(w2, nullptr)};
        for (Var* w : {w1, w2})
        {
            for (const char* i : {"0", "1", "2", "3"})
            {
                body.push_back(k.store(k.b.elem_addr(k.mem(k.addr(w), 0), N(k, i), k.at()), N(k, "1")));
            }
        }
        const Expr* call {k.call(two_w, {k.addr(w1), alias ? k.addr(w1) : k.addr(w2)})};
        body.push_back(k.es(call));
        tail(k, body);
        f->body = k.blkc(body);
        k.done(f);
        if (alias)
        {
            // the 22 checks of the call: the first slot of the leaf against itself (the 9th) fails, the rest hold after it
            k.fail_only("permission", call->loc, 22U, 8U);
        }
    };
    lcv("marr_two_leaves_distinct",
        "two_w(&w1, &w2): two objects of a struct whose only leaf is an array: the first-slot separation row holds",
        [=](Kit& k)
        {
            two_w_main(k, false);
        });
    lc("marr_two_leaves_alias", "two_w(&w1, &w1): the two array leaves are one: one permission error at the call",
       [=](Kit& k)
       {
           two_w_main(k, true);
       });
    // ---- p12's shape (audit round 1, S5): a structure used only through a pointer parameter, no local of its type ----
    lc("marr_only_through_pointers",
       "void f(struct S *p) {} with no local struct S: alloc_struct_<n>_S is emitted all the same",
       [=](Kit& k)
       {
           StructType* S {SA(k)};
           Function* f {k.fn("f", k.vd, {{"p", PS(k, S)}})};
           f->body = k.blk({});
           k.done(f);
       });
    // ---- a nested member (item 3: the chain loc(member(loc(member(t, 1), 0), 1), i)) ----
    lcv("marr_nested",
        "struct T { int x; struct S s; } t: t.s.a[1] written and read; the allocation reaches the nested array",
        [=](Kit& k)
        {
            StructType* S {SA(k)};
            StructType* T {k.strukt("T", {{"x", k.i32}, {"s", k.st(S)}})};
            Function* f {k.fn("f", k.vd)};
            Var* t {k.scell(f, "t", T)};
            const Expr* ts_a {k.mem(k.mem(k.addr(t), 1), 1)};
            std::vector<const Stmt*> body {k.decl(t, nullptr), k.lstore(k.addr(t), 0, N(k, "1")),
                                           k.store(k.b.elem_addr(ts_a, N(k, "1"), k.at()), N(k, "4")),
                                           k.chk(k.eq_c(k.deref(k.b.elem_addr(ts_a, N(k, "1"), k.at())), k.i32, "4"))};
            tail(k, body);
            f->body = k.blkc(body);
            k.done(f);
        });
    // ---- what check_program still refuses (the backstops of the record's table: questions 292-294, 306, by-value structures: part E) ----
    rc("marr_rej_by_value_param", "array elements passed by value",
       [=](Kit& k)
       {
           // struct B { int a[N + 1]; }; void f(struct B s) {}  (questions 295 and 309: array values past kMaxByValueArrayElements elements are M4.5f3)
           StructType* S {k.strukt("B", {{"a", k.prog.array_of(k.i32, kMaxByValueArrayElements + 1U)}})};
           Function* f {k.fn("f", k.vd, {{"s", k.st(S)}})};
           f->body = k.blk({});
           k.done(f);
       });
    rc("marr_rej_by_value_result", "array elements passed by value",
       [=](Kit& k)
       {
           // struct B { int a[N + 1]; }; struct B mk(void);
           StructType* S {k.strukt("B", {{"a", k.prog.array_of(k.i32, kMaxByValueArrayElements + 1U)}})};
           Function* mk {k.fn("mk", k.st(S))};
           k.trusted(mk);
       });
    rc("marr_rej_live_of_a_member_array", "or a `live` of one",
       [=](Kit& k)
       {
           // requires(live(p->a))
           StructType* S {SA(k)};
           Function* f {k.fn("f", k.vd, {{"p", PS(k, S)}})};
           k.req(f, k.b.atom(AtomKind::Live, k.mem(FPs(k, f, 0), 1), 0, k.at()));
           f->body = k.blk({});
           k.done(f);
       });
    rc("marr_rej_owns_range_of_a_member_array", "a range permission atom of something other than a pointer parameter",
       [=](Kit& k)
       {
           // requires(owns_range(p->a, 2)): a part of an array member is M4.5e2's
           StructType* S {SA(k)};
           Function* f {k.fn("f", k.vd, {{"p", PS(k, S)}})};
           k.req(f, k.b.range_atom(AtomKind::OwnsRange, k.mem(FPs(k, f, 0), 1), N(k, "2"), 0, k.at()));
           f->body = k.blk({});
           k.done(f);
       });
    // ---- the decay of an array member passed to a callee's owns_range (item 8) ----
    // void fillr(int *q, int n) requires(owns_range(q, n)) ensures(owns_range(q, n)) {}   void use(void) { struct S s; ...; fillr(s.a, [4|5]); check(s.a[0] == 1 [wrong: not known after the call]); }
    auto range_member_call = [=](Kit& k, const char* count, bool wrong_check) -> const Expr*
    {
        StructType* S {SA(k)};
        Function* r {k.fn("fillr", k.vd, {{"q", k.ptr(k.i32)}, {"n", k.i32}})};
        k.req(r, k.b.range_atom(AtomKind::OwnsRange, FPs(k, r, 0), FPs(k, r, 1), 0, k.at()));
        k.ens(r, k.b.range_atom(AtomKind::OwnsRange, FPs(k, r, 0), FPs(k, r, 1), 0, k.at()));
        r->body = k.blk({});
        k.done(r);
        Function* f {k.fn("use", k.vd)};
        Var* s {k.scell(f, "s", S)};
        std::vector<const Stmt*> body;
        fill(k, s, body, "7");
        const Expr* decay {k.b.text(k.b.elem_addr(k.mem(k.addr(s), 1), N(k, "0"), k.at()), "s.a")};
        const Expr* call {k.call(r, {decay, N(k, count)})};
        body.push_back(k.es(call));
        const Stmt* c {nullptr};
        if (wrong_check)
        {
            c = k.chk(k.eq_c(rd(k, k.addr(s), "0"), k.i32, "1"));
            body.push_back(c);
            k.fail("check", c->loc);
        }
        else
        {
            body.push_back(k.chk(k.eq_c(rdn(k, k.addr(s)), k.i32, "7")));
        }
        f->body = k.blkc(body);
        k.done(f);
        return call;
    };
    lc("marr_range_call",
       "fillr(s.a, 4): the decay of the member bound to a local, e1b's rows over it (the whole array); s.n stays known",
       [=](Kit& k)
       {
           range_member_call(k, "4", false);
       });
    lc("marr_range_call_forgot", "the callee owned s.a and promised nothing about it: s.a[0] == 1 after the call fails",
       [=](Kit& k)
       {
           range_member_call(k, "4", true);
       });
    lc("marr_range_call_too_long",
       "fillr(s.a, 5) over a four-element member: the shape row of the range (longer than its object) fails",
       [=](Kit& k)
       {
           k.fail_only("permission", range_member_call(k, "5", false)->loc, 8U, 0U);
       });
    // ---- sequencing (item 11): the elements of an array member are objects of their own ----
    // void f(int i, int j) requires(0 <= i && i < 4 && 0 <= j && j < 4) { struct S s; s.a[0..3] = ...; s.a[<lhs>] = s.a[<rhs>]++; }
    auto seq_case = [=](Kit& k, bool variable, const Expr** out_inner)
    {
        StructType* S {SA(k)};
        Function* f {k.fn("f", k.vd, {{"i", k.i32}, {"j", k.i32}})};
        k.req(f, k.land(k.land(k.le(N(k, "0"), k.F(f, 0)), k.lt(k.F(f, 0), N(k, "4"))),
                        k.land(k.le(N(k, "0"), k.F(f, 1)), k.lt(k.F(f, 1), N(k, "4")))));
        Var* s {k.scell(f, "s", S)};
        std::vector<const Stmt*> body;
        fill(k, s, body, "7");
        const Expr* lhs {k.b.elem_addr(k.mem(k.addr(s), 1), variable ? k.pv(f, 0) : N(k, "0"), k.at())};
        const Expr* rhs {k.b.elem_addr(k.mem(k.addr(s), 1), variable ? k.pv(f, 1) : N(k, "1"), k.at())};
        const Expr* inner {k.b.inc_dec_store_expr(rhs, true, true, k.at())};
        const Expr* outer {k.b.store_expr(lhs, inner, k.at())};
        body.push_back(k.es(outer));
        f->body = k.blkc(body);
        k.done(f);
        *out_inner = outer;
    };
    lc("marr_seq_distinct_elements",
       "s.a[0] = s.a[1]++: two elements of one member are two objects: no unsequenced row fails",
       [=](Kit& k)
       {
           const Expr* inner {nullptr};
           seq_case(k, false, &inner);
       });
    lc("marr_seq_unknown_indexes",
       "s.a[i] = s.a[j]++ with i and j unknown: the two elements may be one: unsequenced at the later access",
       [=](Kit& k)
       {
           const Expr* inner {nullptr};
           seq_case(k, true, &inner);
           k.fail_only("unsequenced", inner->loc, 1U, 0U);
       });
    // ---- loops (item 10) ----
    // struct S s; s.n = 1; s.a = {7, 7, 7, 7}; int *p = &s.a[1]; long t = 0; int i = 0; while (i < 4) inv 0 <= i <= 4 && t == 7 * i { t = t + s.a[i]; [*p = 1;] i++; } check(t == 28);
    auto sum_local = [=](Kit& k, bool pointer_write)
    {
        StructType* S {SA(k)};
        Function* f {k.fn("f", k.vd)};
        Var* s {k.scell(f, "s", S)};
        Var* p {k.local(f, "p", k.ptr(k.i32))};
        Var* t {k.local(f, "t", k.i64)};
        Var* i {k.local(f, "i", k.i32)};
        std::vector<const Stmt*> body {k.decl(s, nullptr), k.lstore(k.addr(s), 0, N(k, "1"))};
        for (const char* j : {"0", "1", "2", "3"})
        {
            body.push_back(wr(k, k.addr(s), j, "7"));
        }
        body.push_back(k.decl(p, k.b.elem_addr(k.mem(k.addr(s), 1), N(k, "1"), k.at())));
        body.push_back(k.decl(t, k.num(k.i64, "0")));
        body.push_back(k.decl(i, N(k, "0")));
        std::vector<const Stmt*> bs {k.asg(t, k.add(k.v(t), k.cv(k.i64, rd(k, k.addr(s), "0"))))};
        // (the element read is s.a[i]: the index is the variable i)
        bs[0] = k.asg(t, k.add(k.v(t), k.cv(k.i64, k.deref(k.b.elem_addr(k.mem(k.addr(s), 1), k.pn(i), k.at())))));
        if (pointer_write)
        {
            bs.push_back(k.store(k.pn(p), N(k, "1")));
        }
        bs.push_back(k.incS(i));
        const Clause kept {k.cl(k.eq(k.v(t), k.mul(k.cv(k.i64, N(k, "7")), k.cv(k.i64, k.v(i)))))};
        body.push_back(k.whl(k.lt(k.v(i), N(k, "4")), {k.cl(k.range(k.v(i), k.i32, "0", "4")), kept},
                             {k.cl(k.sub(N(k, "4"), k.v(i)))}, k.blkc(bs)));
        body.push_back(k.chk(k.eq_c(k.v(t), k.i64, "28")));
        f->body = k.blkc(body);
        k.done(f);
        if (pointer_write)
        {
            k.fail("invariant-preserved", kept.loc);
        }
    };
    lc("marr_loop_local_sum",
       "a read-only loop over s.a of a structure local: the array leaf is an array place beside the cell's, the values are kept: t == 7 * i holds and t == 28 after the loop",
       [=](Kit& k)
       {
           sum_local(k, false);
       });
    lc("marr_loop_local_pointer_write",
       "the body also stores through p = &s.a[1]: a store through a pointer reaches s.a, its values are not kept: t == 7 * i is not preserved",
       [=](Kit& k)
       {
           sum_local(k, true);
       });
    // audit round 3, S4: a heap unit (a malloc view exists), struct S s; struct S *p = &s; s.a[1] = 5; int i = 0; while (i < 3) inv 0 <= i <= 3 { p->a[1] = i; i++; } [check(s.a[1] == 5);]
    // The pointer local's heap place is its object's cells (the object view); the array leaf is s's own array place, which a store through the pointer reaches.
    auto heap_loop = [=](Kit& k, bool wrong)
    {
        StructType* S {SA(k)};
        heap_alloc_view(k, "malloc", k.i32, false);
        Function* f {k.fn("f", k.vd)};
        Var* s {k.scell(f, "s", S)};
        Var* p {k.local(f, "p", k.ptr(k.st(S)))};
        Var* i {k.local(f, "i", k.i32)};
        std::vector<const Stmt*> body {k.decl(s, nullptr), k.lstore(k.addr(s), 0, N(k, "1"))};
        for (const char* j : {"0", "1", "2", "3"})
        {
            body.push_back(wr(k, k.addr(s), j, "5"));
        }
        body.push_back(k.decl(p, k.addr(s)));
        body.push_back(k.decl(i, N(k, "0")));
        body.push_back(
            k.whl(k.lt(k.v(i), N(k, "3")), {k.cl(k.range(k.v(i), k.i32, "0", "3"))}, {k.cl(k.sub(N(k, "3"), k.v(i)))},
                  k.blkc({k.store(k.b.elem_addr(k.mem(k.pn(p), 1), N(k, "1"), k.at()), k.v(i)), k.incS(i)})));
        const Stmt* c {wrong ? k.chk(k.eq_c(rd(k, k.addr(s), "1"), k.i32, "5")) : nullptr};
        if (wrong)
        {
            body.push_back(c);
        }
        else
        {
            tail(k, body);
        }
        f->body = k.blkc(body);
        k.done(f);
        if (wrong)
        {
            k.fail("check", c->loc);
        }
    };
    lcv("marr_loop_heap_pointer_local",
        "a unit with a malloc view: a loop over a pointer local to a structure local with an array member: the pointer's heap place is its cells, the array leaf is the local's own place",
        [=](Kit& k)
        {
            heap_loop(k, false);
        });
    lc("marr_loop_heap_pointer_local_wrong", "the loop writes p->a[1]: s.a[1] == 5 after it fails",
       [=](Kit& k)
       {
           heap_loop(k, true);
       });
    // void fill(struct S *p) requires(owns(*p)) ensures(owns(*p)) { int i = 0; while (i < 4) inv 0 <= i <= 4 { p->a[i] = i; i++; } }
    lc("marr_loop_param_fill",
       "a loop that writes p->a[i] under owns(*p): the array leaf of the held place is a range place at write, the flags are kept",
       [=](Kit& k)
       {
           StructType* S {SA(k)};
           Function* f {k.fn("fill", k.vd, {{"p", PS(k, S)}})};
           k.req(f, k.b.atom(AtomKind::Owns, FPs(k, f, 0), 0, k.at()));
           k.ens(f, k.b.atom(AtomKind::Owns, FPs(k, f, 0), 0, k.at()));
           Var* i {k.local(f, "i", k.i32)};
           const Stmt* body {
               k.blkc({k.store(k.b.elem_addr(k.mem(k.pv(f, 0), 1), k.pn(i), k.at()), k.v(i)), k.incS(i)})};
           f->body =
               k.blkc({k.decl(i, N(k, "0")), k.whl(k.lt(k.v(i), N(k, "4")), {k.cl(k.range(k.v(i), k.i32, "0", "4"))},
                                                   {k.cl(k.sub(N(k, "4"), k.v(i)))}, body)});
           k.done(f);
       });
    // void keep(struct S *p) requires(owns(*p) && p->a[0] == 5) ensures(owns(*p)) { int i = 0; while (i < 4) inv 0 <= i <= 4 && p->a[0] == 5 { [p->a[0] = 6;] i++; } }
    auto keep_param = [=](Kit& k, bool writes)
    {
        StructType* S {SA(k)};
        Function* f {k.fn("keep", k.vd, {{"p", PS(k, S)}})};
        auto a0 = [&](const Expr* base)
        {
            return k.deref(k.b.elem_addr(k.mem(base, 1), N(k, "0"), k.at()));
        };
        k.req(f, k.land(k.b.atom(AtomKind::Owns, FPs(k, f, 0), 0, k.at()), k.eq_c(a0(FPs(k, f, 0)), k.i32, "5")));
        k.ens(f, k.b.atom(AtomKind::Owns, FPs(k, f, 0), 0, k.at()));
        Var* i {k.local(f, "i", k.i32)};
        std::vector<const Stmt*> bs;
        if (writes)
        {
            bs.push_back(k.store(k.b.elem_addr(k.mem(k.pv(f, 0), 1), N(k, "0"), k.at()), N(k, "6")));
        }
        bs.push_back(k.incS(i));
        const Clause kept {k.cl(k.eq_c(a0(k.pv(f, 0)), k.i32, "5"))};
        f->body =
            k.blkc({k.decl(i, N(k, "0")), k.whl(k.lt(k.v(i), N(k, "4")), {k.cl(k.range(k.v(i), k.i32, "0", "4")), kept},
                                                {k.cl(k.sub(N(k, "4"), k.v(i)))}, k.blkc(bs))});
        k.done(f);
        if (writes)
        {
            k.fail("invariant-preserved", kept.loc);
        }
    };
    // the invariant reads p->a[i] for i up to 4 over a four-element member: out-of-bounds at the invariant's read
    lc("marr_loop_inv_read_outside",
       "while (i < 5) inv p->a[i] >= INT_MIN over int a[4]: the index may be outside p->a: out-of-bounds at the invariant's read",
       [=](Kit& k)
       {
           StructType* S {SA(k)};
           Function* f {k.fn("outside", k.vd, {{"p", PS(k, S)}})};
           k.req(f, k.b.atom(AtomKind::Reads, FPs(k, f, 0), 0, k.at()));
           k.ens(f, k.b.atom(AtomKind::Reads, FPs(k, f, 0), 0, k.at()));
           Var* i {k.local(f, "i", k.i32)};
           const Expr* r {k.deref(k.b.elem_addr(k.mem(k.b.text(k.pv(f, 0), "p"), 1), k.pn(i), k.at()))};
           const Clause inv {k.cl(k.ge_c(r, k.i32, "-2147483648"))};
           f->body = k.blkc(
               {k.decl(i, N(k, "0")), k.whl(k.lt(k.v(i), N(k, "5")), {k.cl(k.range(k.v(i), k.i32, "0", "5")), inv},
                                            {k.cl(k.sub(N(k, "5"), k.v(i)))}, k.blkc({k.incS(i)}))});
           k.done(f);
           k.fail_only("out-of-bounds", r->loc, 2U, 1U);
       });
    lc("marr_loop_param_inv_read",
       "p->a[0] read in the invariant of a loop that never writes it: the range place keeps the values and frames the read",
       [=](Kit& k)
       {
           keep_param(k, false);
       });
    lc("marr_loop_param_inv_write", "the body writes p->a[0]: the invariant p->a[0] == 5 is not preserved",
       [=](Kit& k)
       {
           keep_param(k, true);
       });
    // long total(struct S *p) requires(reads(*p)) ensures(reads(*p)) { long t = 0; int i = 0; while (i < 4) inv 0 <= i <= 4 { t = t + p->a[i]; i++; } return t; }
    lc("marr_loop_param_reads", "a read-only loop over p->a under reads(*p): a range place at the atom's amount",
       [=](Kit& k)
       {
           StructType* S {SA(k)};
           Function* f {k.fn("total", k.i64, {{"p", PS(k, S)}})};
           k.req(f, k.b.atom(AtomKind::Reads, FPs(k, f, 0), 0, k.at()));
           k.ens(f, k.b.atom(AtomKind::Reads, FPs(k, f, 0), 0, k.at()));
           Var* t {k.local(f, "t", k.i64)};
           Var* i {k.local(f, "i", k.i32)};
           const Stmt* body {k.blkc(
               {k.asg(t, k.cv(k.i64, k.deref(k.b.elem_addr(k.mem(k.pv(f, 0), 1), k.pn(i), k.at())))), k.incS(i)})};
           f->body = k.blkc({k.decl(t, k.num(k.i64, "0")), k.decl(i, N(k, "0")),
                             k.whl(k.lt(k.v(i), N(k, "4")), {k.cl(k.range(k.v(i), k.i32, "0", "4"))},
                                   {k.cl(k.sub(N(k, "4"), k.v(i)))}, body),
                             k.ret(k.v(t))});
           k.done(f);
       });
    // ---- a struct global with an array leaf (item 9; the leaf's rows in program_start, the facts, main holding it, a callee owning it) ----
    // struct S g = {n, {vals...}} (the rest of the array zero); the leaves in object-view order: n, then the array a
    auto gstruct = [=](Kit& k, StructType* S, const char* name, const char* n,
                       const std::vector<const char*>& vals) -> Global*
    {
        Global* g {k.prog.add_global(name, k.st(S), k.at(), nullptr)};
        GlobalLeaf ln;
        ln.kind = GlobalInit::Value;
        ln.init = k.num(k.i32, n);
        ln.value = n;
        ln.loc = k.at();
        g->leaves.push_back(ln);
        GlobalLeaf la;
        la.kind = GlobalInit::Value;
        la.rest = ArrayRest::Zero;
        la.loc = k.at();
        for (const char* v : vals)
        {
            GlobalLeaf e;
            e.kind = GlobalInit::Value;
            e.init = k.num(k.i32, v);
            e.value = v;
            e.loc = k.at();
            la.elems.push_back(e);
        }
        g->leaves.push_back(la);
        return g;
    };
    auto gea = [=](Kit& k, const Global* g, const Expr* i)
    {
        return k.b.elem_addr(k.mem(k.b.global_addr(g, k.at()), 1), i, k.at());
    };
    // struct S g = {7, {1, 2}}; int main() { check(g.n == 7 && g.a[0] == 1 && g.a[1] == 2 && g.a[2] == 0 [wrong: == 1]); g.a[1] = 5; check(g.a[1] == 5 && g.a[0] == 1); return 0; }
    auto gmain = [=](Kit& k, const char* wrong)
    {
        StructType* S {SA(k)};
        Global* g {gstruct(k, S, "g", "7", {"1", "2"})};
        Function* m {k.fn("main", k.i32)};
        const Expr* last {k.deref(gea(k, g, N(k, "2")))};
        const Stmt* c1 {
            k.chk(k.land(k.land(k.eq_c(k.leaf(k.b.global_addr(g, k.at()), 0), k.i32, "7"),
                                k.eq_c(k.deref(gea(k, g, N(k, "0"))), k.i32, "1")),
                         k.land(k.eq_c(k.deref(gea(k, g, N(k, "1"))), k.i32, "2"), k.eq_c(last, k.i32, wrong))))};
        const Stmt* w {k.store(gea(k, g, N(k, "1")), N(k, "5"))};
        const Stmt* c2 {k.chk(k.land(k.eq_c(k.deref(gea(k, g, N(k, "1"))), k.i32, "5"),
                                     k.eq_c(k.deref(gea(k, g, N(k, "0"))), k.i32, "1")))};
        std::vector<const Stmt*> body {c1, w, c2};
        tail(k, body);
        body.push_back(k.ret(N(k, "0")));
        m->body = k.blk(body);
        k.done(m);
        k.prog.set_entry(m);
        if (std::string(wrong) != "0")
        {
            k.fail("check", c1->loc);
        }
    };
    lcv("marr_global_read_write",
        "struct S g = {7, {1, 2}} held by main from program_start: the array leaf's row group (the permissions, the flags, the listed values, the zero rest), then a write and a read back",
        [=](Kit& k)
        {
            gmain(k, "0");
        });
    // struct F { int a[4]; int n; } f = {{1, 2}, 7} (the array member FIRST) and struct W { int a[4]; } w = {{3}} (no cell leaf): main reads and writes them
    auto garray_leaf = [=](Kit& k, const std::vector<const char*>& vals) -> GlobalLeaf
    {
        GlobalLeaf la;
        la.kind = GlobalInit::Value;
        la.rest = ArrayRest::Zero;
        la.loc = k.at();
        for (const char* v : vals)
        {
            GlobalLeaf e;
            e.kind = GlobalInit::Value;
            e.init = k.num(k.i32, v);
            e.value = v;
            e.loc = k.at();
            la.elems.push_back(e);
        }
        return la;
    };
    auto garr_first = [=](Kit& k, bool wrong)
    {
        StructType* F {k.strukt("F", {{"a", k.prog.array_of(k.i32, 4U)}, {"n", k.i32}})};
        Global* g {k.prog.add_global("f", k.st(F), k.at(), nullptr)};
        g->leaves.push_back(garray_leaf(k, {"1", "2"}));
        GlobalLeaf ln;
        ln.kind = GlobalInit::Value;
        ln.init = k.num(k.i32, "7");
        ln.value = "7";
        ln.loc = k.at();
        g->leaves.push_back(ln);
        Function* m {k.fn("main", k.i32)};
        const Expr* a2 {k.deref(k.b.elem_addr(k.mem(k.b.global_addr(g, k.at()), 0), N(k, "2"), k.at()))};
        const Stmt* c {k.chk(
            k.land(k.eq_c(k.leaf(k.b.global_addr(g, k.at()), 1), k.i32, "7"), k.eq_c(a2, k.i32, wrong ? "1" : "0")))};
        m->body = k.blk({c, k.ret(N(k, "0"))});
        k.done(m);
        k.prog.set_entry(m);
        if (wrong)
        {
            k.fail("check", c->loc);
        }
    };
    lc("marr_global_array_first",
       "struct F { int a[4]; int n; } f = {{1, 2}, 7}: the cell row comes first whatever the member index, then the array leaf's group",
       [=](Kit& k)
       {
           garr_first(k, false);
       });
    lc("marr_global_array_first_wrong", "the zero rest of the array-first leaf: f.a[2] == 1 fails",
       [=](Kit& k)
       {
           garr_first(k, true);
       });
    auto garr_only = [=](Kit& k, bool wrong)
    {
        StructType* W {k.strukt("W", {{"a", k.prog.array_of(k.i32, 4U)}})};
        Global* g {k.prog.add_global("w", k.st(W), k.at(), nullptr)};
        g->leaves.push_back(garray_leaf(k, {"3"}));
        Function* m {k.fn("main", k.i32)};
        const Expr* a0 {k.deref(k.b.elem_addr(k.mem(k.b.global_addr(g, k.at()), 0), N(k, "0"), k.at()))};
        const Stmt* c {k.chk(k.eq_c(a0, k.i32, wrong ? "4" : "3"))};
        m->body = k.blk({c, k.ret(N(k, "0"))});
        k.done(m);
        k.prog.set_entry(m);
        if (wrong)
        {
            k.fail("check", c->loc);
        }
    };
    lc("marr_global_array_only",
       "struct W { int a[4]; } w = {{3}}: no cell leaf: the structure's row is the liveness cell alone, then the array's group",
       [=](Kit& k)
       {
           garr_only(k, false);
       });
    lc("marr_global_array_only_wrong", "w.a[0] == 4 fails",
       [=](Kit& k)
       {
           garr_only(k, true);
       });
    lc("marr_global_zero_rest_wrong",
       "the zero rest of the leaf is exactly the elements past the list: g.a[2] == 1 fails",
       [=](Kit& k)
       {
           gmain(k, "1");
       });
    // struct S g = {1, {7, 7, 7, 7}}; int main() { long t = 0; int i = 0; while (i < 4) inv 0 <= i <= 4 && t == 7 * i { t = t + g.a[i]; i++; } check(t == 28); return 0; }  [mode 1: g.a[i] = i in the body: the values are not kept]
    auto gsum = [=](Kit& k, bool writes)
    {
        StructType* S {SA(k)};
        Global* g {gstruct(k, S, "g", "1", {"7", "7", "7", "7"})};
        Function* m {k.fn("main", k.i32)};
        Var* t {k.local(m, "t", k.i64)};
        Var* i {k.local(m, "i", k.i32)};
        std::vector<const Stmt*> bs {k.asg(t, k.add(k.v(t), k.cv(k.i64, k.deref(gea(k, g, k.pn(i))))))};
        if (writes)
        {
            bs.push_back(k.store(gea(k, g, k.pn(i)), N(k, "1")));
        }
        bs.push_back(k.incS(i));
        const Clause kept {k.cl(k.eq(k.v(t), k.mul(k.cv(k.i64, N(k, "7")), k.cv(k.i64, k.v(i)))))};
        m->body = k.blkc({k.decl(t, k.num(k.i64, "0")), k.decl(i, N(k, "0")),
                          k.whl(k.lt(k.v(i), N(k, "4")), {k.cl(k.range(k.v(i), k.i32, "0", "4")), kept},
                                {k.cl(k.sub(N(k, "4"), k.v(i)))}, k.blkc(bs)),
                          k.chk(k.eq_c(k.v(t), k.i64, "28")), k.ret(N(k, "0"))});
        k.done(m);
        k.prog.set_entry(m);
        if (writes)
        {
            k.fail("invariant-preserved", kept.loc);
        }
    };
    lc("marr_global_loop_sum",
       "a read-only loop over g.a in main: an array place for the leaf, the values are kept: t == 7 * i holds, t == 28 after",
       [=](Kit& k)
       {
           gsum(k, false);
       });
    lc("marr_global_loop_write", "the body also writes g.a[i]: the values are not kept, t == 7 * i is not preserved",
       [=](Kit& k)
       {
           gsum(k, true);
       });
    // void set(void) requires(owns(g)) ensures(owns(g) && g.a[1] == 9) { g.a[1] = 9; }   int main() { set(); check(g.a[1] == 9 && g.n == 7); return 0; }
    lcv("marr_global_owns_call",
        "a callee owning the struct global (its cell and its array leaf) writes g.a[1]; main holds the global from program_start and calls it",
        [=](Kit& k)
        {
            StructType* S {SA(k)};
            Global* g {gstruct(k, S, "g", "7", {"1", "2"})};
            Function* set {k.fn("set", k.vd)};
            k.req(set, k.b.global_atom(AtomKind::Owns, k.b.global_addr(g, k.at()), 0, k.at()));
            k.ens(set, k.land(k.b.global_atom(AtomKind::Owns, k.b.global_addr(g, k.at()), 0, k.at()),
                              k.eq_c(k.deref(gea(k, g, N(k, "1"))), k.i32, "9")));
            set->body = k.blk({k.store(gea(k, g, N(k, "1")), N(k, "9"))});
            k.done(set);
            Function* m {k.fn("main", k.i32)};
            std::vector<const Stmt*> body {k.es(k.call(set, {})),
                                           k.chk(k.eq_c(k.deref(gea(k, g, N(k, "1"))), k.i32, "9"))};
            tail(k, body);
            body.push_back(k.ret(N(k, "0")));
            m->body = k.blk(body);
            k.done(m);
            k.prog.set_entry(m);
        });
    lc("marr_global_owns_call_forgot",
       "set() owns the whole of g and promises only g.a[1] == 9: g.n == 7 after the call is not known, the check fails",
       [=](Kit& k)
       {
           StructType* S {SA(k)};
           Global* g {gstruct(k, S, "g", "7", {"1", "2"})};
           Function* set {k.fn("set", k.vd)};
           k.req(set, k.b.global_atom(AtomKind::Owns, k.b.global_addr(g, k.at()), 0, k.at()));
           k.ens(set, k.b.global_atom(AtomKind::Owns, k.b.global_addr(g, k.at()), 0, k.at()));
           set->body = k.blk({});
           k.done(set);
           Function* m {k.fn("main", k.i32)};
           const Stmt* c {k.chk(k.eq_c(k.leaf(k.b.global_addr(g, k.at()), 0), k.i32, "7"))};
           m->body = k.blk({k.es(k.call(set, {})), c, k.ret(N(k, "0"))});
           k.done(m);
           k.prog.set_entry(m);
           k.fail("check", c->loc);
       });
    // audit round 3, S1: struct S g = {7, {1, 2}}; void bump(void) requires(owns(g)) ensures(owns(g)) {}   int main() { int i = 0; while (i < 3) inv 0 <= i <= 3 dec 3 - i { bump(); i++; } [check(g.n == 7);] return 0; }
    // The loop's invariant carries the global's liveness (its cell place's), so the callee's `owns(g)` is held at every iteration; the twin relies on a value the callee may have changed.
    auto gloop_call = [=](Kit& k, bool array_only, bool wrong)
    {
        Global* g {nullptr};
        if (array_only)
        {
            StructType* W {k.strukt("W", {{"a", k.prog.array_of(k.i32, 4U)}})};
            g = k.prog.add_global("w", k.st(W), k.at(), nullptr);
            g->leaves.push_back(garray_leaf(k, {"3"}));
        }
        else
        {
            g = gstruct(k, SA(k), "g", "7", {"1", "2"});
        }
        Function* bump {k.fn("bump", k.vd)};
        k.req(bump, k.b.global_atom(AtomKind::Owns, k.b.global_addr(g, k.at()), 0, k.at()));
        k.ens(bump, k.b.global_atom(AtomKind::Owns, k.b.global_addr(g, k.at()), 0, k.at()));
        bump->body = k.blk({});
        k.done(bump);
        Function* m {k.fn("main", k.i32)};
        Var* i {k.local(m, "i", k.i32)};
        const Stmt* chk {wrong ? k.chk(k.eq_c(k.leaf(k.b.global_addr(g, k.at()), 0), k.i32, "7")) : nullptr};
        std::vector<const Stmt*> body {
            k.decl(i, N(k, "0")), k.whl(k.lt(k.v(i), N(k, "3")), {k.cl(k.range(k.v(i), k.i32, "0", "3"))},
                                        {k.cl(k.sub(N(k, "3"), k.v(i)))}, k.blkc({k.es(k.call(bump, {})), k.incS(i)}))};
        if (wrong)
        {
            body.push_back(chk);
        }
        else
        {
            tail(k, body);
        }
        body.push_back(k.ret(N(k, "0")));
        m->body = k.blkc(body);
        k.done(m);
        k.prog.set_entry(m);
        if (wrong)
        {
            k.fail("check", chk->loc);
        }
    };
    lcv("marr_global_loop_call",
        "main's loop calls a callee that owns the structure global (cell and array leaf): the invariant carries the global's liveness, the call's owns(g) is held at every iteration",
        [=](Kit& k)
        {
            gloop_call(k, false, false);
        });
    lc("marr_global_loop_call_wrong",
       "the same loop, then check(g.n == 7): the callee owns g and promises nothing about it, the check fails",
       [=](Kit& k)
       {
           gloop_call(k, false, true);
       });
    lcv("marr_global_loop_call_array_only",
        "the same for struct W { int a[4]; } w (no cell leaf): the liveness-only place carries the global's liveness",
        [=](Kit& k)
        {
            gloop_call(k, true, false);
        });
    // void inc(struct S *p, int i) requires(owns(*p) && 0 <= i && i < 4 && p->a[i] < 100) ensures(owns(*p) && p->a[i] == old(p->a[i]) + 1) { p->a[i] = p->a[i] + 1; }
    auto make_inc = [=](Kit& k, StructType* S, bool wrong) -> Function*
    {
        Function* f {k.fn("inc", k.vd, {{"p", PS(k, S)}, {"i", k.i32}})};
        auto elem_c = [&](const Expr* idx)
        {
            return k.deref(k.b.elem_addr(k.mem(FPs(k, f, 0), 1), idx, k.at()));
        };
        k.req(f, k.land(k.b.atom(AtomKind::Owns, FPs(k, f, 0), 0, k.at()),
                        k.land(k.land(k.le(N(k, "0"), k.F(f, 1)), k.lt(k.F(f, 1), N(k, "4"))),
                               k.lt(elem_c(k.F(f, 1)), N(k, "100")))));
        k.ens(f, k.land(k.b.atom(AtomKind::Owns, FPs(k, f, 0), 0, k.at()),
                        k.eq(elem_c(k.F(f, 1)), k.add(k.old(elem_c(k.F(f, 1))), N(k, wrong ? "2" : "1")))));
        const Expr* addr {k.b.elem_addr(k.mem(k.pv(f, 0), 1), k.pv(f, 1), k.at())};
        f->body =
            k.blk({k.store(addr, k.add(k.deref(k.b.elem_addr(k.mem(k.pv(f, 0), 1), k.pv(f, 1), k.at())), N(k, "1")))});
        k.done(f);
        if (wrong)
        {
            k.fail("postcondition", f->end_loc);
        }
        return f;
    };
    lc("marr_elem_old",
       "an element read in a requires and, under old, in an ensures: the index inside [0, N) frames it by owns(*p); the body adds 1",
       [=](Kit& k)
       {
           make_inc(k, SA(k), false);
       });
    lc("marr_elem_old_wrong", "the ensures promises + 2 where the body adds 1: the postcondition fails",
       [=](Kit& k)
       {
           make_inc(k, SA(k), true);
       });
    // int bad(struct S *p, int i) requires(owns(*p) && p->a[i] == 0) ensures(owns(*p)) {}: the read is framed only when i is inside the array: the contract is not well formed (permission at the read)
    lc("marr_elem_unframed",
       "p->a[i] read in a requires with no bound on i: the read is framed only by an index inside the array: permission at the read, in the contract",
       [=](Kit& k)
       {
           StructType* S {SA(k)};
           Function* f {k.fn("bad", k.vd, {{"p", PS(k, S)}, {"i", k.i32}})};
           const Expr* r {k.deref(k.b.elem_addr(k.mem(FPs(k, f, 0), 1), k.F(f, 1), k.at()))};
           k.req(f, k.land(k.b.atom(AtomKind::Owns, FPs(k, f, 0), 0, k.at()), k.eq_c(r, k.i32, "0")));
           k.ens(f, k.b.atom(AtomKind::Owns, FPs(k, f, 0), 0, k.at()));
           f->body = k.blk({});
           k.done(f);
           k.fail("permission", r->loc);
       });
    lc("marr_local_initializer",
       "struct S s = {7, {1, 2}}: ArrayZero, the two listed elements, the cell: the unlisted elements are 0",
       [=](Kit& k)
       {
           init_local(k, "0");
       });
    lc("marr_local_initializer_wrong", "the same: s.a[2] == 1 does not hold, the check fails",
       [=](Kit& k)
       {
           init_local(k, "1");
       });
}

// The three views of a struct's leaves and the spelling of the new places, checked directly (they have no Viper): returns the number of failures, printed as they are found.
int check_array_views()
{
    int bad {0};
    auto expect = [&](bool ok, const std::string& what)
    {
        if (!ok)
        {
            std::fprintf(stderr, "FAIL (array views) %s\n", what.c_str());
            ++bad;
        }
    };
    Kit k;
    // struct T { int p; int a[2]; }; struct S { struct T t; int *q[2]; bool b; };
    StructType* T {k.strukt("T", {{"p", k.i32}, {"a", k.prog.array_of(k.i32, 2U)}})};
    StructType* S {k.strukt("S", {{"t", k.st(T)}, {"q", k.prog.array_of(k.ptr(k.i32), 2U)}, {"b", k.bl}})};
    // the old view: the poison, an array member one leaf of the array's type, never flagged as an array leaf
    const std::vector<StructLeaf> old_view {struct_leaves(*S, "s")};
    expect(old_view.size() == 4U, "the old view of S has four leaves");
    if (old_view.size() == 4U)
    {
        expect(old_view[1U].type.is_array() && !old_view[1U].array && old_view[1U].elem == -1 &&
                   old_view[1U].text == "s.t.a",
               "the old view keeps s.t.a as one leaf of the array's type");
        expect(old_view[2U].type.is_array() && old_view[2U].text == "s.q",
               "the old view keeps s.q as one leaf of the array's type");
    }
    // the object view: an array leaf with its element type and count
    const std::vector<StructLeaf> object_view {struct_object_leaves(*S, "s")};
    expect(object_view.size() == 4U, "the object view of S has four leaves");
    if (object_view.size() == 4U)
    {
        expect(object_view[0U].path == std::vector<int> {0, 0} && !object_view[0U].array,
               "the object view's first leaf is the cell s.t.p");
        expect(object_view[1U].array && object_view[1U].count == 2U && object_view[1U].element == k.i32 &&
                   object_view[1U].path == std::vector<int> {0, 1} && object_view[1U].text == "s.t.a",
               "the object view has the array leaf s.t.a of two ints");
        expect(object_view[2U].array && object_view[2U].count == 2U && object_view[2U].element == k.ptr(k.i32) &&
                   object_view[2U].path == std::vector<int> {1},
               "the object view has the array leaf s.q of two int pointers");
        expect(!object_view[3U].array && object_view[3U].type == k.bl, "the object view's last leaf is the cell s.b");
    }
    // the value view: every element a leaf of its own, in index order
    const std::vector<StructLeaf> value_view {struct_value_leaves(*S, "s")};
    expect(value_view.size() == 6U, "the value view of S has six leaves: p, a[0], a[1], q[0], q[1], b");
    if (value_view.size() == 6U)
    {
        expect(value_view[1U].elem == 0 && value_view[1U].text == "s.t.a[0]" && value_view[1U].type == k.i32 &&
                   value_view[1U].path == std::vector<int> {0, 1},
               "the value view's second leaf is s.t.a[0]");
        expect(value_view[2U].elem == 1 && value_view[2U].text == "s.t.a[1]",
               "the value view's third leaf is s.t.a[1]");
        expect(value_view[3U].elem == 0 && value_view[3U].type == k.ptr(k.i32) && value_view[3U].text == "s.q[0]",
               "the value view's fourth leaf is s.q[0]");
        expect(value_view[0U].elem == -1 && value_view[5U].elem == -1, "a cell leaf has no element index");
    }
    // a struct without an array member: the three views are the same list
    StructType* U {k.strukt("U", {{"x", k.i32}, {"v", k.st(k.strukt("V", {{"y", k.bl}}))}})};
    const std::vector<StructLeaf> a {struct_leaves(*U, "u")};
    const std::vector<StructLeaf> b {struct_object_leaves(*U, "u")};
    const std::vector<StructLeaf> c {struct_value_leaves(*U, "u")};
    bool same {a.size() == b.size() && a.size() == c.size()};
    for (size_t i {0U}; same && i < a.size(); ++i)
    {
        same = a[i].path == b[i].path && a[i].path == c[i].path && a[i].text == b[i].text && a[i].text == c[i].text &&
               a[i].type == b[i].type && a[i].type == c[i].type && !b[i].array && c[i].elem == -1;
    }
    expect(same, "for a struct without an array member the three views are the same list");

    // the spelling of the places of the new chains (place_text, member_place, elem_place)
    Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(S))}})};
    k.done(f);
    Var* s {k.scell(f, "s", S)};
    Global* g {k.prog.add_global("g", k.prog.array_of(k.i32, 3U), k.at(), nullptr)};
    const Expr* i {k.b.text(k.num(k.i32, "0"), "i")};
    const Expr* p {k.b.text(k.pv(f, 0), "p")};
    // s.t.a[i]: AddrOf s -> member t -> member a -> element
    const Expr* s_t_a {k.mem(k.mem(k.addr(s), 0), 1)};
    expect(place_text(*k.b.elem_addr(s_t_a, i, k.at())) == "s.t.a[i]", "place_text of s.t.a[i]");
    expect(place_text(*s_t_a) == "s.t.a", "place_text of the member array s.t.a");
    // p->t.a[2] with a constant index spelled by its value, and p->q[i]
    const Expr* p_t_a {k.mem(k.mem(p, 0), 1)};
    expect(k.b.elem_addr(p_t_a, k.num(k.i32, "2"), k.at())->c_text == "&p->t.a[2]", "c_text of &p->t.a[2]");
    expect(k.deref(k.b.elem_addr(k.mem(p, 1), i, k.at()))->c_text == "p->q[i]",
           "the read of p->q[i] is spelled p->q[i]");
    // the element's pointer type: a pointer to the array's element type, for a member array and a global array alike
    expect(k.b.elem_addr(p_t_a, i, k.at())->type == k.ptr(k.i32), "the address of p->t.a[i] is an int pointer");
    expect(k.b.elem_addr(k.b.global_addr(g, k.at()), i, k.at())->type == k.ptr(k.i32),
           "the address of g[i] is an int pointer");
    expect(place_text(*k.b.elem_addr(k.b.global_addr(g, k.at()), i, k.at())) == "g[i]", "place_text of g[i]");
    expect(k.deref(k.b.elem_addr(k.b.global_addr(g, k.at()), i, k.at()))->c_text == "g[i]",
           "the read of g[i] is spelled g[i]");
    // ElemValue: the element type, the spelling
    Function* mk {k.fn("mk", k.st(T))};
    k.trusted(mk);
    const Expr* ev {k.b.elem_value(k.b.text(k.call(mk, {}), "mk()"), 1, k.num(k.i32, "1"), k.at())};
    expect(ev->type == k.i32 && ev->index == 1 && ev->c_text == "mk().a[1]", "ElemValue's type and spelling");
    return bad;
}

// ---- structure values with an array member (decision 0063 item 13, question 291; part E) ----
// struct T { int n; int a[3]; }: the value view is the tuple (n, a[0], a[1], a[2]); a by-value parameter is four formals `p_s__0_m0, p_s__0_m1_e0, ...`, a result `res_m0, res_m1_e0, ...`, a copy between
// objects is the cell stores and `ArrayCopy` of the array leaf (the lowering's rows, then `copy_arr_int` under the guard where a place is reached through a pointer).  Each rule has a case that
// verifies (with its vacuity twin: `check(false)` at the end fails) and a twin that fails for one reason.
void define_struct_array_values()
{
    auto N = [](Kit& k, const char* v)
    {
        return k.num(k.i32, v);
    };
    auto TV = [](Kit& k)
    {
        return k.strukt("T", {{"n", k.i32}, {"a", k.prog.array_of(k.i32, 3U)}});
    };
    auto tail = [](Kit& k, std::vector<const Stmt*>& body)
    {
        if (k.vac_tail)
        {
            const Stmt* v {k.chk(k.no())};
            body.push_back(v);
            k.vac_loc = v->loc;
        }
    };
    auto lcv = [](const char* name, const char* doc, const std::function<void(Kit&)>& build)
    {
        lc(name, doc, build);
        lc(std::string(name) + "_vacuity",
           "the same with check(false) after everything: it fails (the rows are consistent)",
           [build](Kit& k)
           {
               k.vac_tail = true;
               build(k);
               k.fail("check", k.vac_loc);
           });
    };
    auto elem = [=](Kit& k, const Expr* base, const char* i)
    {
        return k.deref(k.b.elem_addr(k.mem(base, 1), N(k, i), k.at()));
    };
    auto set = [=](Kit& k, const Expr* base, const char* i, const char* v)
    {
        return k.store(k.b.elem_addr(k.mem(base, 1), N(k, i), k.at()), N(k, v));
    };
    // `t` with every member written: t.n = n; t.a = {a0, a1, a2}
    auto fill = [=](Kit& k, Var* t, std::vector<const Stmt*>& body, const char* n, const char* a0, const char* a1,
                    const char* a2)
    {
        body.push_back(k.decl(t, nullptr));
        body.push_back(k.lstore(k.addr(t), 0, N(k, n)));
        body.push_back(set(k, k.addr(t), "0", a0));
        body.push_back(set(k, k.addr(t), "1", a1));
        body.push_back(set(k, k.addr(t), "2", a2));
    };
    // the whole value of the object `&v`: (struct T){v.n, v.a[0], v.a[1], v.a[2]}
    auto whole = [=](Kit& k, const StructType* T, const Expr* base)
    {
        return k.b.aggregate(k.st(T), {k.leaf(base, 0), elem(k, base, "0"), elem(k, base, "1"), elem(k, base, "2")},
                             k.at());
    };

    // int get(struct T t) requires(t.a[1] == 5) ensures(r == t.a[1] + 1) { t.a[1] = t.a[1] + 1; return t.a[1]; }
    // int use(void) { struct T s; s.n = 3; s.a = {4, 5, 6}; int r = get(s); check(r == 6); check(s.a[1] == 5 [wrong: == 6]); return r; }
    auto by_value = [=](Kit& k, bool wrong, const Stmt** at)
    {
        StructType* T {TV(k)};
        Function* get {k.fn("get", k.i32, {{"t", k.st(T)}})};
        get->mark_address_taken(get->param_var(0));
        const Expr* entry {k.b.elem_value(k.F(get, 0), 1, N(k, "1"), k.at())};
        k.req(get, k.eq(entry, N(k, "5")));
        k.ens(get, k.eq(k.R(get), k.add(k.b.elem_value(k.F(get, 0), 1, N(k, "1"), k.at()), N(k, "1"))));
        const Expr* tp {k.addr(get->param_var(0))};
        get->body = k.blkc({k.store(k.b.elem_addr(k.mem(tp, 1), N(k, "1"), k.at()), k.add(elem(k, tp, "1"), N(k, "1"))),
                            k.ret(elem(k, tp, "1"))});
        k.done(get);
        Function* use {k.fn("use", k.i32)};
        Var* s {k.scell(use, "s", T)};
        Var* r {k.local(use, "r", k.i32)};
        std::vector<const Stmt*> body;
        fill(k, s, body, "3", "4", "5", "6");
        body.push_back(k.decl(r, k.call(get, {whole(k, T, k.addr(s))})));
        body.push_back(k.chk(k.eq(k.v(r), N(k, "6"))));
        const Stmt* c {k.chk(k.eq(elem(k, k.addr(s), "1"), N(k, wrong ? "6" : "5")))};
        body.push_back(c);
        tail(k, body);
        body.push_back(k.ret(k.v(r)));
        use->body = k.blkc(body);
        k.done(use);
        *at = c;
    };
    lcv("earr_val_param",
        "a by-value struct with an array member: the formals are the leaves, the callee changes its copy's element, the contract reads the entry value, the caller's object is unchanged",
        [=](Kit& k)
        {
            const Stmt* c {nullptr};
            by_value(k, false, &c);
        });
    lc("earr_val_param_caller_unchanged",
       "check(s.a[1] == 6) after get(s): the callee changed only its copy, s.a[1] is still 5",
       [=](Kit& k)
       {
           const Stmt* c {nullptr};
           by_value(k, true, &c);
           k.fail("check", c->loc);
       });
    // void f(struct T t) requires(t.a[2] == 9) { check(t.a[2] == 9); }   void use(void) { struct T s; s.n = 0; s.a = {1, 2, 9 [wrong: 8]}; f(s); }
    auto requires_case = [=](Kit& k, bool wrong)
    {
        StructType* T {TV(k)};
        Function* f {k.fn("f", k.vd, {{"t", k.st(T)}})};
        f->mark_address_taken(f->param_var(0));
        k.req(f, k.eq(k.b.elem_value(k.F(f, 0), 1, N(k, "2"), k.at()), N(k, "9")));
        std::vector<const Stmt*> fb {k.chk(k.eq(elem(k, k.addr(f->param_var(0)), "2"), N(k, "9")))};
        tail(k, fb);
        f->body = k.blkc(fb);
        k.done(f);
        Function* use {k.fn("use", k.vd)};
        Var* s {k.scell(use, "s", T)};
        std::vector<const Stmt*> body;
        fill(k, s, body, "0", "1", "2", wrong ? "8" : "9");
        const Expr* call {k.call(f, {whole(k, T, k.addr(s))})};
        body.push_back(k.es(call));
        use->body = k.blkc(body);
        k.done(use);
        if (wrong)
        {
            k.fail("precondition", call->loc);
        }
    };
    (void)requires_case;
    lcv("earr_val_requires",
        "s.a[2] of a by-value parameter in a requires: the caller passes a value that satisfies it, the callee's copy holds it",
        [=](Kit& k)
        {
            requires_case(k, false);
        });
    lc("earr_val_requires_wrong", "the caller passes s.a[2] == 8 where the requires needs 9: precondition at the call",
       [=](Kit& k)
       {
           requires_case(k, true);
       });
    // void f(struct T t, int i) requires(t.a[i] == 0) {}: the index is not known to be inside the array, and the contract's well-formedness asserts the bound (out-of-bounds), as every mode does
    lc("earr_val_requires_unbounded",
       "t.a[i] of a by-value parameter in a requires with no bound on i: out-of-bounds in the contract",
       [=](Kit& k)
       {
           StructType* T {TV(k)};
           Function* f {k.fn("f", k.vd, {{"t", k.st(T)}, {"i", k.i32}})};
           f->mark_address_taken(f->param_var(0));
           const Expr* ev {k.b.elem_value(k.F(f, 0), 1, k.F(f, 1), k.at())};
           k.req(f, k.eq(ev, N(k, "0")));
           f->body = k.blk({});
           k.done(f);
           k.fail("out-of-bounds", ev->loc);
       });
    // struct T mk(int x) ensures(r.n == x && r.a[1] == 7) { struct T v; v.n = x; v.a = {1, 7, 3}; return v; }
    // int use(void) { struct T t; t = mk(2); check(t.n == 2 && t.a[1] == 7 [wrong: 8]); check(mk(2).a[1] == 7); return 0; }
    auto result_case = [=](Kit& k, bool wrong)
    {
        StructType* T {TV(k)};
        Function* mk {k.fn("mk", k.st(T), {{"x", k.i32}})};
        k.ens(mk, k.land(k.eq(k.b.field(k.R(mk), 0, k.at()), k.F(mk, 0)),
                         k.eq(k.b.elem_value(k.R(mk), 1, N(k, "1"), k.at()), N(k, "7"))));
        Var* v {k.scell(mk, "v", T)};
        std::vector<const Stmt*> mb;
        fill(k, v, mb, "0", "1", "7", "3");
        mb[1] = k.lstore(k.addr(v), 0, k.pv(mk, 0));
        mb.push_back(k.ret(whole(k, T, k.addr(v))));
        mk->body = k.blkc(mb);
        k.done(mk);
        Function* use {k.fn("use", k.i32)};
        Var* t {k.scell(use, "t", T)};
        const Stmt* c {k.chk(
            k.land(k.eq(k.leaf(k.addr(t), 0), N(k, "2")), k.eq(elem(k, k.addr(t), "1"), N(k, wrong ? "8" : "7"))))};
        Var* z {k.local(use, "z", k.i32)};
        std::vector<const Stmt*> body {k.decl(t, nullptr), k.store(k.addr(t), k.call(mk, {N(k, "2")})), c,
                                       k.decl(z, k.b.elem_value(k.call(mk, {N(k, "2")}), 1, N(k, "1"), k.at())),
                                       k.chk(k.eq(k.v(z), N(k, "7")))};
        tail(k, body);
        body.push_back(k.ret(N(k, "0")));
        use->body = k.blkc(body);
        k.done(use);
        if (wrong)
        {
            k.fail("check", c->loc);
        }
    };
    lcv("earr_val_result",
        "a by-value result with an array member: return v reads every element into the result leaves, the call's results are stored into t's cells, mk(2).a[1] is an element of the result",
        [=](Kit& k)
        {
            result_case(k, false);
        });
    lc("earr_val_result_wrong", "check(t.a[1] == 8) after t = mk(2): the ensures says r.a[1] == 7",
       [=](Kit& k)
       {
           result_case(k, true);
       });
    // struct T mk(void) ensures(r.a[0] == 1); int pick(int i) { return mk().a[i]; }: i is not known to be inside the array
    lc("earr_val_result_index_outside", "mk().a[i] with an unconstrained i: out-of-bounds at the element",
       [=](Kit& k)
       {
           StructType* T {TV(k)};
           Function* mk {k.fn("mk", k.st(T))};
           k.trusted(mk);
           Function* pick {k.fn("pick", k.i32, {{"i", k.i32}})};
           const Expr* ev {k.b.elem_value(k.call(mk, {}), 1, k.pv(pick, 0), k.at())};
           pick->body = k.blk({k.ret(ev)});
           k.done(pick);
           k.fail_only("out-of-bounds", ev->loc, 1U, 0U);
       });
    // int g(struct T t) ensures(r == old(t.a[2]) && r == t.a[2]) { return t.a[2]; }: old over an element of a by-value parameter
    lc("earr_val_old",
       "old(t.a[2]) of a by-value parameter in an ensures: the entry value, the callee returns its copy's element",
       [=](Kit& k)
       {
           StructType* T {TV(k)};
           Function* g {k.fn("g", k.i32, {{"t", k.st(T)}})};
           g->mark_address_taken(g->param_var(0));
           const Expr* ev {k.b.elem_value(k.F(g, 0), 1, N(k, "2"), k.at())};
           k.ens(g, k.land(k.eq(k.R(g), k.old(ev)), k.eq(k.R(g), k.b.elem_value(k.F(g, 0), 1, N(k, "2"), k.at()))));
           g->body = k.blk({k.ret(elem(k, k.addr(g->param_var(0)), "2"))});
           k.done(g);
       });
    // struct P { int *a[2]; }; void f(struct P s) requires(s.a[1] == NULL) { check(s.a[1] == NULL); }: an element that is a pointer is a pointer leaf (WF at entry)
    lc("earr_val_pointer_elements",
       "a by-value struct whose array member holds pointers: the element leaves are pointer formals, s.a[1] == NULL in a requires and in the callee's copy",
       [=](Kit& k)
       {
           StructType* P {k.strukt("P", {{"a", k.prog.array_of(k.ptr(k.i32), 2U)}})};
           Function* f {k.fn("f", k.vd, {{"s", k.st(P)}})};
           f->mark_address_taken(f->param_var(0));
           const Type ip {k.ptr(k.i32)};
           k.req(f, k.eq(k.b.elem_value(k.F(f, 0), 0, N(k, "1"), k.at()), k.nul(ip)));
           f->body = k.blk(
               {k.chk(k.eq(k.deref(k.b.elem_addr(k.mem(k.addr(f->param_var(0)), 0), N(k, "1"), k.at())), k.nul(ip)))});
           k.done(f);
       });
    // struct P { int *a[2]; int n; }; struct P mk(int *x) requires(live(x)) ensures(live(x) && r.a[0] == x) { struct P v; v.a[0] = x; v.a[1] = NULL; v.n = 1; return v; }
    // void use(void) { int x; struct P t; t = mk(&x); check(t.a[0] == &x [wrong: t.a[1] == &x]); }
    // The pointer elements are pointer leaves of the result (WF in the contract of the method, a lifetime row at their use by the caller's store and the return's reads).
    auto pointer_result = [=](Kit& k, int mode)
    {
        const Type ip {k.ptr(k.i32)};
        StructType* P {k.strukt("P", {{"a", k.prog.array_of(ip, 2U)}, {"n", k.i32}})};
        Function* mk {k.fn("mk", k.st(P), {{"x", ip}})};
        auto xs = [&]()
        {
            return k.b.text(k.F(mk, 0), "x");
        };
        k.req(mk, k.b.atom(AtomKind::Live, xs(), 0, k.at()));
        const Expr* promise {k.land(k.b.atom(AtomKind::Live, xs(), 0, k.at()),
                                    k.eq(k.b.elem_value(k.R(mk), 0, N(k, "0"), k.at()), k.F(mk, 0)))};
        if (mode != 2)
        {
            promise = k.land(promise, k.eq(k.b.elem_value(k.R(mk), 0, N(k, "1"), k.at()), k.nul(ip)));
        }
        k.ens(mk, promise);
        Var* v {k.scell(mk, "v", P)};
        const Expr* vp {k.addr(v)};
        auto pe = [&](const char* i)
        {
            return k.deref(k.b.elem_addr(k.mem(vp, 0), N(k, i), k.at()));
        };
        mk->body =
            k.blkc({k.decl(v, nullptr), k.store(k.b.elem_addr(k.mem(vp, 0), N(k, "0"), k.at()), k.pv(mk, 0)),
                    k.store(k.b.elem_addr(k.mem(vp, 0), N(k, "1"), k.at()), k.nul(ip)), k.lstore(vp, 1, N(k, "1")),
                    k.ret(k.b.aggregate(k.st(P), {pe("0"), pe("1"), k.leaf(vp, 1)}, k.at()))});
        k.done(mk);
        Function* use {k.fn("use", k.vd)};
        Var* x {k.cell(use, "x", k.i32)};
        Var* t {k.scell(use, "t", P)};
        const Expr* tp {k.addr(t)};
        const Stmt* c {
            k.chk(k.eq(k.deref(k.b.elem_addr(k.mem(tp, 0), N(k, mode == 1 ? "1" : "0"), k.at())), k.addr(x)))};
        const Expr* mkcall {k.call(mk, {k.addr(x)})};
        const Stmt* st {k.store(tp, mkcall)};
        std::vector<const Stmt*> ub {k.decl(x, nullptr), k.decl(t, nullptr), st, c};
        tail(k, ub);
        use->body = k.blkc(ub);
        k.done(use);
        if (mode == 1)
        {
            k.fail("check", c->loc);
        }
        else if (mode == 2)
        {
            k.fail_only("lifetime", mkcall->loc, 2U, 1U);
        }
    };
    lcv("earr_val_pointer_result",
        "a by-value result whose array member holds pointers: the element leaves are pointer results, r.a[0] == x in the ensures, the caller stores them into t and compares t.a[0] with &x",
        [=](Kit& k)
        {
            pointer_result(k, 0);
        });
    lc("earr_val_pointer_result_wrong", "check(t.a[1] == &x): the callee promises r.a[1] == NULL, the check fails",
       [=](Kit& k)
       {
           pointer_result(k, 1);
       });
    lc("earr_val_pointer_result_unknown_element",
       "the callee promises nothing about r.a[1]: the caller's store of the result uses a pointer that may point to an ended object (lifetime, at the call)",
       [=](Kit& k)
       {
           pointer_result(k, 2);
       });
    // ---- the copy between objects (item 13: the cell stores, the array leaf by ArrayCopy) ----
    // struct T s, t; s = {3, {1, 2, 3}}; t = s; check(t.n == 3 && t.a[2] == 3); t.a[0] = 9; check(s.a[0] == 1 && t.a[0] == 9); t = t;
    auto copy_locals = [=](Kit& k, int mode)
    {
        // mode 0: positive; mode 1: s.a[2] never written (uninitialized at the copy); mode 2: check(t.a[2] == 4) (a wrong value)
        StructType* T {TV(k)};
        Function* f {k.fn("f", k.vd)};
        Var* s {k.scell(f, "s", T)};
        Var* t {k.scell(f, "t", T)};
        std::vector<const Stmt*> body;
        body.push_back(k.decl(s, nullptr));
        body.push_back(k.lstore(k.addr(s), 0, N(k, "3")));
        body.push_back(set(k, k.addr(s), "0", "1"));
        body.push_back(set(k, k.addr(s), "1", "2"));
        if (mode != 1)
        {
            body.push_back(set(k, k.addr(s), "2", "3"));
        }
        body.push_back(k.decl(t, nullptr));
        body.push_back(k.lstore(k.addr(t), 0, k.leaf(k.addr(s), 0)));
        const Stmt* copy {k.b.array_copy(k.mem(k.addr(t), 1), k.mem(k.addr(s), 1), k.at())};
        body.push_back(copy);
        const Stmt* c {k.chk(
            k.land(k.eq(k.leaf(k.addr(t), 0), N(k, "3")), k.eq(elem(k, k.addr(t), "2"), N(k, mode == 2 ? "4" : "3"))))};
        body.push_back(c);
        if (mode == 0)
        {
            body.push_back(set(k, k.addr(t), "0", "9"));
            body.push_back(
                k.chk(k.land(k.eq(elem(k, k.addr(s), "0"), N(k, "1")), k.eq(elem(k, k.addr(t), "0"), N(k, "9")))));
            body.push_back(k.b.array_copy(k.mem(k.addr(t), 1), k.mem(k.addr(t), 1), k.at()));
            body.push_back(k.chk(k.eq(elem(k, k.addr(t), "0"), N(k, "9"))));
        }
        tail(k, body);
        f->body = k.blkc(body);
        k.done(f);
        if (mode == 1)
        {
            k.fail("uninitialized", copy->loc);
        }
        else if (mode == 2)
        {
            k.fail("check", c->loc);
        }
    };
    lcv("earr_copy_locals",
        "t = s between two structure locals: the cell, then the array leaf by copy_arr_int (the source's rows, the destination's rows); the copy is independent; t = t does nothing",
        [=](Kit& k)
        {
            copy_locals(k, 0);
        });
    lc("earr_copy_unwritten",
       "s.a[2] is never written: copying s reads an element without a value, uninitialized at the copy",
       [=](Kit& k)
       {
           copy_locals(k, 1);
       });
    lc("earr_copy_wrong_value", "check(t.a[2] == 4) after the copy of s.a = {1, 2, 3}: the destination holds 3",
       [=](Kit& k)
       {
           copy_locals(k, 2);
       });
    // void cp(struct T *p, struct T *q) requires(owns(*p) && reads(*q)) ensures(owns(*p) && reads(*q) && p->a[2] == q->a[2]) { *p = *q; }   [wrong: requires(reads(*p) && reads(*q))]
    auto copy_ptrs = [=](Kit& k, bool dst_reads)
    {
        StructType* T {TV(k)};
        auto ps = [&](Function* f, int i)
        {
            return k.b.text(k.F(f, i), f->params[static_cast<size_t>(i)].name);
        };
        const Type pt {k.ptr(k.st(T))};
        Function* cp {k.fn("cp", k.vd, {{"p", pt}, {"q", pt}})};
        const AtomKind pk {dst_reads ? AtomKind::Reads : AtomKind::Owns};
        k.req(cp, k.land(k.b.atom(pk, ps(cp, 0), 0, k.at()), k.b.atom(AtomKind::Reads, ps(cp, 1), 1, k.at())));
        k.ens(cp, k.land(k.land(k.b.atom(pk, ps(cp, 0), 0, k.at()), k.b.atom(AtomKind::Reads, ps(cp, 1), 1, k.at())),
                         k.eq(k.deref(k.b.elem_addr(k.mem(ps(cp, 0), 1), N(k, "2"), k.at())),
                              k.deref(k.b.elem_addr(k.mem(ps(cp, 1), 1), N(k, "2"), k.at())))));
        const Stmt* cell {k.store(k.mem(k.pv(cp, 0), 0), k.deref(k.mem(k.pv(cp, 1), 0)))};
        const Stmt* arr {k.b.array_copy(k.mem(k.pv(cp, 0), 1), k.mem(k.pv(cp, 1), 1), k.at())};
        cp->body = k.blkc({cell, arr});
        k.done(cp);
        if (dst_reads)
        {
            k.fail("permission", cell->loc);
        }
    };
    lc("earr_copy_through_pointers",
       "*p = *q with owns(*p) and reads(*q): the cell, then the array leaf under the guard (the two first slots differ), the postcondition reads the copied element",
       [=](Kit& k)
       {
           copy_ptrs(k, false);
       });
    lc("earr_copy_destination_reads",
       "the same with reads(*p): the destination is only readable, permission at the write of its cell",
       [=](Kit& k)
       {
           copy_ptrs(k, true);
       });
    // void self(struct T *p) requires(owns(*p)) ensures(owns(*p) && p->a[1] == old(p->a[1])) { *p = *p; }: an exact self-copy
    lc("earr_copy_self_through_pointer",
       "*p = *p: the guard d != s is false, the array copy does nothing, the elements are unchanged",
       [=](Kit& k)
       {
           StructType* T {TV(k)};
           const Type pt {k.ptr(k.st(T))};
           Function* self {k.fn("self", k.vd, {{"p", pt}})};
           auto ps = [&]()
           {
               return k.b.text(k.F(self, 0), "p");
           };
           k.req(self, k.b.atom(AtomKind::Owns, ps(), 0, k.at()));
           const Expr* a1 {k.deref(k.b.elem_addr(k.mem(ps(), 1), N(k, "1"), k.at()))};
           k.ens(self, k.land(k.b.atom(AtomKind::Owns, ps(), 0, k.at()), k.eq(a1, k.old(a1))));
           const Stmt* cell {k.store(k.mem(k.pv(self, 0), 0), k.deref(k.mem(k.pv(self, 0), 0)))};
           const Stmt* arr {k.b.array_copy(k.mem(k.pv(self, 0), 1), k.mem(k.pv(self, 0), 1), k.at())};
           self->body = k.blkc({cell, arr});
           k.done(self);
       });
    // a structure of exactly kMaxByValueArrayElements elements is a by-value parameter: struct B { int a[N]; } void f(struct B b) requires(b.a[N-1] == 1) { check(b.a[N-1] == 1); }
    // (the limit, question 309: N + 1 is a rejection case above; the worst shape, a structure in and out by value with a filling caller, takes 25 s at N = 7)
    lc("earr_val_limit_elements",
       "a structure that holds exactly the most array elements a value may hold is passed by value: the entry copy writes every element, the contract reads the last",
       [=](Kit& k)
       {
           const unsigned last {static_cast<unsigned>(kMaxByValueArrayElements) - 1U};
           StructType* B {k.strukt("B", {{"a", k.prog.array_of(k.i32, kMaxByValueArrayElements)}})};
           Function* f {k.fn("f", k.vd, {{"b", k.st(B)}})};
           f->mark_address_taken(f->param_var(0));
           const std::string idx {std::to_string(last)};
           k.req(f, k.eq(k.b.elem_value(k.F(f, 0), 0, N(k, idx.c_str()), k.at()), N(k, "1")));
           f->body = k.blk({k.chk(
               k.eq(k.deref(k.b.elem_addr(k.mem(k.addr(f->param_var(0)), 0), N(k, idx.c_str()), k.at())), N(k, "1")))});
           k.done(f);
       });
}

} // namespace

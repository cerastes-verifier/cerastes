// (included by selftest.cpp) Arrays in structures and in static storage, M4.5e1e1 Part A (decision 0063 item 4: plumbing, nothing accepted): `Type::Array` is representable as the type of a
// StructMember and of a Global; the three views of a struct's leaves (struct_leaves, the old view, keeps the array member as one leaf of the array's type: the poison;
// struct_object_leaves gives an array leaf; struct_value_leaves one leaf per element); `Global::elems` and `GlobalLeaf::elems` hold the initial values of an array; the statements
// `ArrayZero` and `ArrayCopy` and the expression `ElemValue` are built by the Builder.  check_program refuses every one of them in every position (the rejection cases below), and
// lowering stops a program that reaches it anyway with an internal error (the `ru` cases, which lower a refused program).  Part B (a global array) and part C (an array member) lift
// the refusals one by one and turn each case into a lowering case.
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
    rc("arr_store_rej_member_scalar_array", "which M4.5e1e1 part C lifts",
       [=](Kit& k)
       {
           S_arr(k);
       });
    rc("arr_store_rej_member_pointer_array", "which M4.5e1e1 part C lifts",
       [=](Kit& k)
       {
           // struct S { int *a[2]; };
           k.strukt("S", {{"a", k.prog.array_of(k.ptr(k.i32), 2U)}});
       });
    rc("arr_store_rej_member_array_of_structs", "which M4.5e1e1 part C lifts",
       [=](Kit& k)
       {
           // struct T { int x; }; struct S { struct T a[2]; };
           StructType* T {k.strukt("T", {{"x", k.i32}})};
           k.strukt("S", {{"a", k.prog.array_of(k.st(T), 2U)}});
       });
    rc("arr_store_rej_member_array_in_a_nested_struct", "which M4.5e1e1 part C lifts",
       [=](Kit& k)
       {
           // struct T { int a[3]; }; struct S { struct T t; int n; };
           StructType* T {k.strukt("T", {{"a", A3(k)}})};
           k.strukt("S", {{"t", k.st(T)}, {"n", k.i32}});
       });

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
    rc("arr_store_rej_global_array_unknown_element_zero_rest", "unknown initial value although the rest of the array is zero",
       [=](Kit& k)
       {
           Global* g {k.prog.add_global("g", A3(k), k.at(), nullptr)};
           GlobalLeaf one;
           one.kind = GlobalInit::Unknown;
           g->elems = {one};
       });
    rc("arr_store_rej_global_array_known_element_unknown_rest", "known initial value although the rest of the array is not known",
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
    rc("arr_store_rej_elem_of_a_member_array_through_a_pointer", "an element of an array member",
       [=](Kit& k)
       {
           // struct S { int a[3]; int n; }; int f(struct S *p) { return p->a[1]; }
           StructType* S {S_arr(k)};
           Function* f {k.fn("f", k.i32, {{"p", k.ptr(k.st(S))}})};
           f->body = k.blk({k.ret(k.deref(k.b.elem_addr(k.mem(k.pv(f, 0), 0), N(k, "1"), k.at())))});
           k.done(f);
       });
    rc("arr_store_rej_elem_of_a_member_array_of_a_local", "an element of an array member",
       [=](Kit& k)
       {
           // void f(void) { struct S s; s.a[1] = 5; }
           StructType* S {S_arr(k)};
           Function* f {k.fn("f", k.vd)};
           Var* s {k.scell(f, "s", S)};
           f->body =
               k.blkc({k.decl(s, nullptr), k.store(k.b.elem_addr(k.mem(k.addr(s), 0), N(k, "1"), k.at()), N(k, "5"))});
           k.done(f);
       });
    rc("arr_store_rej_elem_of_a_member_array_in_a_contract", "an element of an array member",
       [=](Kit& k)
       {
           // requires(p->a[1] == 0)
           StructType* S {S_arr(k)};
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(S))}})};
           k.req(f, k.eq(k.deref(k.b.elem_addr(k.mem(k.F(f, 0), 0), N(k, "1"), k.at())), N(k, "0")));
           f->body = k.blk({});
           k.done(f);
       });
    rc("arr_store_rej_atom_on_a_member_array", "is not a bool, an integer, a pointer or a struct",
       [=](Kit& k)
       {
           // owns(p->a): the member is refused where it is declared, and no atom on it is accepted either
           StructType* S {S_arr(k)};
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(S))}})};
           k.req(f, k.b.atom(AtomKind::Owns, k.mem(k.F(f, 0), 0), 0, k.at()));
           f->body = k.blk({});
           k.done(f);
       });

    // ---- the new statements and the new expression ----
    rc("arr_store_rej_array_zero_statement", "an array zero-fill statement",
       [=](Kit& k)
       {
           StructType* S {S_arr(k)};
           Function* f {k.fn("f", k.vd)};
           Var* s {k.scell(f, "s", S)};
           f->body = k.blkc({k.decl(s, nullptr), k.b.array_zero(k.mem(k.addr(s), 0), k.at())});
           k.done(f);
       });
    rc("arr_store_rej_array_zero_of_a_local_array", "an array zero-fill statement",
       [=](Kit& k)
       {
           // even the address of an array local (whose zero fill is Var::zero_filled) is not an ArrayZero's operand yet
           Function* f {k.fn("f", k.vd)};
           Var* a {k.cell(f, "a", A3(k))};
           f->body = k.blkc({k.decl(a, nullptr), k.b.array_zero(k.addr(a), k.at())});
           k.done(f);
       });
    rc("arr_store_rej_array_copy_statement", "an array copy statement",
       [=](Kit& k)
       {
           StructType* S {S_arr(k)};
           Function* f {k.fn("f", k.vd)};
           Var* s {k.scell(f, "s", S)};
           Var* t {k.scell(f, "t", S)};
           f->body = k.blkc({k.decl(s, nullptr), k.decl(t, nullptr),
                             k.b.array_copy(k.mem(k.addr(t), 0), k.mem(k.addr(s), 0), k.at())});
           k.done(f);
       });
    // a function that returns a struct with an array member, and the element of its result
    auto elem_value_of_call = [=](Kit& k, Function*& mk, StructType*& S)
    {
        S = S_arr(k);
        mk = k.fn("mk", k.st(S));
        k.trusted(mk);
    };
    rc("arr_store_rej_elem_value_in_code", "an element of an array member of a struct value",
       [=](Kit& k)
       {
           // int f(void) { return mk().a[1]; }
           Function* mk {nullptr};
           StructType* S {nullptr};
           elem_value_of_call(k, mk, S);
           Function* f {k.fn("f", k.i32)};
           f->body = k.blk({k.ret(k.b.elem_value(k.call(mk, {}), 0, N(k, "1"), k.at()))});
           k.done(f);
       });
    rc("arr_store_rej_elem_value_in_a_contract", "an element of an array member of a struct value",
       [=](Kit& k)
       {
           // requires(s.a[1] == 0) of a by-value parameter `struct S s`
           StructType* S {S_arr(k)};
           Function* f {k.fn("f", k.vd, {{"s", k.st(S)}})};
           k.req(f, k.eq(k.b.elem_value(k.F(f, 0), 0, N(k, "1"), k.at()), N(k, "0")));
           f->body = k.blk({});
           k.done(f);
       });

    // ---- the guards behind the refusals: lowering stops a refused program that reaches it (exit 3 in the driver) ----
    ru("arr_store_guard_a_struct_object_with_an_array_member", "reached a site that treats every leaf as a cell",
       [=](Kit& k)
       {
           // void f(void) { struct S s; s.n = 1; }: the allocation of `s` takes its leaves in the old view
           StructType* S {S_arr(k)};
           Function* f {k.fn("f", k.vd)};
           Var* s {k.scell(f, "s", S)};
           f->body = k.blkc({k.decl(s, nullptr), k.lstore(k.addr(s), 1, N(k, "1"))});
           k.done(f);
       });
    ru("arr_store_guard_field_of_a_value", "reached a walk of its members that counts every member as one leaf",
       [=](Kit& k)
       {
           // int f(void) { return mk().n; }: the position of member 1 in the tuple of leaves (leaf_span) would read a[1]
           StructType* S {S_arr(k)};
           Function* mk {k.fn("mk", k.st(S))};
           k.trusted(mk);
           Function* f {k.fn("f", k.i32)};
           f->body = k.blk({k.ret(k.b.field(k.call(mk, {}), 1, k.at()))});
           k.done(f);
       });
    ru("arr_store_guard_pointer_to_a_struct_with_an_array_member", "reached a walk of its members that counts every member as one leaf",
       [=](Kit& k)
       {
           // void f(struct S *p) {}: WF_S of the parameter writes `len(member(p, k)) == 1` for every member (struct_lens)
           StructType* S {S_arr(k)};
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(S))}})};
           f->body = k.blk({});
           k.done(f);
       });
    ru("arr_store_guard_array_zero", "an array zero-fill or copy statement reached lowering",
       [=](Kit& k)
       {
           StructType* S {S_arr(k)};
           Function* f {k.fn("f", k.vd)};
           Var* s {k.scell(f, "s", S)};
           f->body = k.blkc({k.decl(s, nullptr), k.b.array_zero(k.mem(k.addr(s), 0), k.at())});
           k.done(f);
       });
    ru("arr_store_guard_array_copy", "an array zero-fill or copy statement reached lowering",
       [=](Kit& k)
       {
           StructType* S {S_arr(k)};
           Function* f {k.fn("f", k.vd)};
           Var* s {k.scell(f, "s", S)};
           Var* t {k.scell(f, "t", S)};
           f->body = k.blkc({k.decl(s, nullptr), k.decl(t, nullptr),
                             k.b.array_copy(k.mem(k.addr(t), 0), k.mem(k.addr(s), 0), k.at())});
           k.done(f);
       });
    ru("arr_store_guard_elem_value", "an element of a struct value's array member reached lowering",
       [=](Kit& k)
       {
           StructType* S {S_arr(k)};
           Function* mk {k.fn("mk", k.st(S))};
           k.trusted(mk);
           Function* f {k.fn("f", k.i32)};
           f->body = k.blk({k.ret(k.b.elem_value(k.call(mk, {}), 0, N(k, "1"), k.at()))});
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
    auto garr = [](Kit& k, const char* name, unsigned long long n, const std::vector<const char*>& vals, bool unknown = false,
                   const Function* owner = nullptr) -> Global*
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
    lcv("garr_main_read_write", "main holds g[4] = {1, 2} from program_start: the listed prefix, the zero rest, then a write and a read back",
        [=](Kit& k)
        {
            Global* g {garr(k, "g", 4U, {"1", "2"})};
            Function* m {k.fn("main", k.i32)};
            const Stmt* c1 {k.chk(k.land(k.land(k.eq_c(grd(k, g, N(k, "0")), k.i32, "1"), k.eq_c(grd(k, g, N(k, "1")), k.i32, "2")),
                                          k.land(k.eq_c(grd(k, g, N(k, "2")), k.i32, "0"), k.eq_c(grd(k, g, N(k, "3")), k.i32, "0"))))};
            const Stmt* w {gwr(k, g, N(k, "1"), N(k, "5"))};
            const Stmt* c2 {k.chk(k.land(k.eq_c(grd(k, g, N(k, "1")), k.i32, "5"), k.land(k.eq_c(grd(k, g, N(k, "0")), k.i32, "1"), k.eq_c(grd(k, g, N(k, "2")), k.i32, "0"))))};
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
    lc("garr_owns_call", "main owns g from program_start: put() takes owns(g) and gives it back with g[1] == 9", [=](Kit& k) { put_main(k, false); });
    lc("garr_owns_call_forgot", "put() owns the whole of g and promises only g[1] == 9: g[0] == 1 after the call fails", [=](Kit& k) { put_main(k, true); });
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
    lc("garr_reads_call", "peek() borrows g at a fraction and returns it: main reads g[0] and g[2] == 0 after", [=](Kit& k) { peek_main(k, false); });
    lc("garr_reads_write", "a write under reads(g): permission at the store (the atom is a fraction)", [=](Kit& k) { peek_main(k, true); });
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
    lc("garr_call_without_permission", "a function with no atom on g calls put(), which needs owns(g): permission at the call",
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
    lc("garr_alias_trap", "two(&g[1]) with owns(*p) && owns(g): the callee would own g[1] twice: one permission error at the call", [=](Kit& k) { alias_main(k, true); });
    lc("garr_alias_distinct", "two(&x) of a distinct local: no overlap with g, no error", [=](Kit& k) { alias_main(k, false); });

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
        m->body = k.blkc({k.decl(p, gat(k, g, N(k, "1"))), k.decl(sv, k.num(k.i64, "0")), k.decl(i, N(k, "0")),
                          k.whl(k.lt(k.v(i), N(k, "3")), {k.cl(bound), kept}, {k.cl(k.sub(N(k, "3"), k.v(i)))}, k.blkc(bs)), c,
                          k.ret(N(k, "0"))});
        k.done(m);
        k.prog.set_entry(m);
        if (mode == 1)
        {
            k.fail("invariant-preserved", kept.loc);
        }
    };
    lc("garr_loop_sum", "a read-only loop over g[3] = {7, 7, 7} in main: the values are kept, s == 7 * i holds and s == 21 after the loop", [=](Kit& k) { sum_main(k, 0); });
    lc("garr_loop_pointer_write", "the body also stores through p = &g[1]: a store through a pointer reaches g, its values are not kept, s == 7 * i is not preserved", [=](Kit& k) { sum_main(k, 1); });
    // int g[4] = {1}; int main() { int i = 0; while (i < 4) inv 0 <= i <= 4 { g[i] = i; i++; } check(i == 4); check(g[2] == 0 [wrong: g[2] == 2 is not known]) }
    lc("garr_loop_fill", "a fill loop over g[4] in main: the loop writes the array (no values kept), the flags are kept, g[1] is readable after it",
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
                             k.whl(k.lt(k.v(i), N(k, "4")), {k.cl(k.range(k.v(i), k.i32, "0", "4"))}, {k.cl(k.sub(N(k, "4"), k.v(i)))}, body), c, r,
                             k.ret(N(k, "0"))});
           k.done(m);
           k.prog.set_entry(m);
       });
    // a loop whose body reads g[i] for i <= 4: the index may be outside g (the bound row of the element read inside the loop)
    lc("garr_loop_read_outside", "while (i <= 4) { s = g[i]; ... } over int g[4]: out-of-bounds at the read in the body",
       [=](Kit& k)
       {
           Global* g {garr(k, "g", 4U, {"1"})};
           Function* m {k.fn("main", k.i32)};
           Var* i {k.local(m, "i", k.i32)};
           Var* sv {k.local(m, "s", k.i32)};
           const Expr* r {grd(k, g, k.pn(i))};
           const Stmt* body {k.blkc({k.asg(sv, r), inc(k, i)})};
           m->body = k.blkc({k.decl(sv, N(k, "0")), k.decl(i, N(k, "0")),
                             k.whl(k.le(k.v(i), N(k, "4")), {k.cl(k.range(k.v(i), k.i32, "0", "5"))}, {k.cl(k.sub(N(k, "5"), k.v(i)))}, body),
                             k.ret(N(k, "0"))});
           k.done(m);
           k.prog.set_entry(m);
           k.fail("out-of-bounds", r->loc);
       });
    // void zero() requires(owns(g)) ensures(owns(g) && g[0] == 0) { int i = 0; while (i < 4) inv 0 <= i <= 4 { g[i] = 0; i++; } }  (a function that holds g through its atom: a range place)
    lc("garr_loop_callee", "a loop in a function that holds g through owns(g): a range place over the bound local, the loop writes the array",
       [=](Kit& k)
       {
           Global* g {garr(k, "g", 4U, {"1"})};
           Function* f {k.fn("zero", k.vd)};
           k.req(f, gatom(k, AtomKind::Owns, g, 0));
           k.ens(f, gatom(k, AtomKind::Owns, g, 0));
           Var* i {k.local(f, "i", k.i32)};
           const Stmt* body {k.blkc({gwr(k, g, k.pn(i), N(k, "0")), inc(k, i)})};
           f->body = k.blkc({k.decl(i, N(k, "0")),
                             k.whl(k.lt(k.v(i), N(k, "4")), {k.cl(k.range(k.v(i), k.i32, "0", "4"))}, {k.cl(k.sub(N(k, "4"), k.v(i)))}, body)});
           k.done(f);
       });

    // int total() requires(reads(g)) ensures(reads(g)) { long s = 0; int i = 0; while (i < 3) inv 0 <= i <= 3 { s = s + g[i]; i++; } return 0; }  (a loop that READS g in a function holding it through reads(g))
    lc("garr_loop_reads_callee", "a read-only loop in a function that borrows g through reads(g): a range place at the atom's amount",
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
                             k.whl(k.lt(k.v(i), N(k, "3")), {k.cl(k.range(k.v(i), k.i32, "0", "3"))}, {k.cl(k.sub(N(k, "3"), k.v(i)))}, body),
                             k.ret(N(k, "0"))});
           k.done(f);
       });
    // the same for a `static` local array of its own function (the implicit owns atom)
    lc("garr_loop_static_local", "int total() { static int g[3] = {7, 7, 7}; loop reading g[i] }: the implicit owns(g) is a range place",
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
                             k.whl(k.lt(k.v(i), N(k, "3")), {k.cl(k.range(k.v(i), k.i32, "0", "3"))}, {k.cl(k.sub(N(k, "3"), k.v(i)))}, body),
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
            const Type at {args[t]->type.is_ptr() && args[t]->type.pointee->is_array() ? k.ptr(*args[t]->type.pointee->element) : args[t]->type};
            a.push_back(k.b.forall_arg(id, static_cast<int>(t), at, args[t]->c_text.empty() ? "x" : args[t]->c_text, k.at()));
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
    lc("garr_forall_check", "check(forall nonneg(g, 4)) over a global array main holds: the bound row, the marked rows, the flags, then the fact", [=](Kit& k) { forall_main(k, 0); });
    lc("garr_forall_check_false_fact", "g[2] is -3: every row holds, the fact fails (check)", [=](Kit& k) { k.fail("check", forall_main(k, 1)->loc); });
    lc("garr_forall_check_outside", "the range is five elements of a four-element array: out-of-bounds at the quantifier's site",
       [=](Kit& k) { k.fail("out-of-bounds", forall_main(k, 2)->expr->loc, 1U, "", "outside `g`"); });

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
    lc("garr_range_call", "fill(g, 4): the decay of g bound to a local, e1b's rows over it (the whole array), g[0] readable after", [=](Kit& k) { range_call_main(k, "4"); });
    lc("garr_range_call_too_long", "fill(g, 5) over a four-element array: the shape row of the range (the range is longer than its object) fails",
       [=](Kit& k) { k.fail_only("permission", range_call_main(k, "5")->loc, 8U, 0U); });

    // ---- item 9 (a `static` local array): its implicit atom ----
    // int counter() { static int g[2] = {0, 0}; g[0] = 5; return g[0]; }   int main() { check(counter() == 5); return 0; }
    lc("garr_static_local", "a static local array's implicit owns atom: counter() takes and returns it; main holds it from program_start and calls it",
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
        const Stmt* c {k.chk(k.land(k.eq_c(k.deref(k.pn(two->param_var(0))), k.i32, wrong ? "2" : "1"), k.eq_c(grd(k, g, N(k, "1")), k.i32, "2")))};
        two->body = k.blk({w1, w2, c});
        k.done(two);
        if (wrong)
        {
            k.fail("check", c->loc);
        }
    };
    lc("garr_callee_cell_and_array", "a callee owning *p and g writes both and reads them back: the cell and the array are distinct objects", [=](Kit& k) { two_body(k, false); });
    lc("garr_callee_cell_and_array_wrong", "the same claiming *p == 2: the check fails (the facts are not inconsistent)", [=](Kit& k) { two_body(k, true); });

    // int g[3] = {1, 2, 3}; int *gp = g; int main() { check(gp == &g[0]); check(gp[0] == 1); return 0; }  (item 9: the decay as an initial value is the address of element 0)
    lc("garr_addr_target", "a pointer global initialized with the decay of an array global holds loc(GB_g(), 0): gp[1]'s element is g[1]",
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
        k.req(f, k.land(k.ge_c(FP(k, f, 1), k.i32, "0"), k.b.range_atom(AtomKind::ReadsRange, FP(k, f, 0), FP(k, f, 1), 0, k.at())));
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
    lc("garr_pure_check", "check(sum_to(g, 3) <= ...) over a global array main holds: the rows over the bound local, then the fact", [=](Kit& k) { const Expr* a {nullptr}; pure_main(k, "3", 0, &a); });
    lc("garr_pure_check_false_fact", "check(sum_to(g, 3) < 0): the rows hold, the fact is not provable (check)", [=](Kit& k) { const Expr* a {nullptr}; k.fail("check", pure_main(k, "3", 1, &a)->loc); });
    lc("garr_pure_check_past", "sum_to(g, 4) over a three-element array: the callee's range line is not given (precondition at the application)",
       [=](Kit& k) { const Expr* a {nullptr}; pure_main(k, "4", 0, &a); k.fail_only("precondition", a->loc, 5U, 3U); });

    // ---- item 10 with item 7: a `forall` and a pure application over a named global array in a loop invariant ----
    // int main() { int i = 0; while (i < 3) inv 0 <= i <= 3 && [forall nonneg(g, 3) | sum_to(g, 3) <= 6442450941] { i++; } return 0; }
    auto inv_main = [=](Kit& k, bool with_forall, bool bad_count) -> const Stmt*
    {
        Global* g {garr(k, "g", 3U, {"1", "2", "3"})};
        Function* sum {with_forall ? nullptr : mk_sum_g(k)};
        Function* m {k.fn("main", k.i32)};
        Var* i {k.local(m, "i", k.i32)};
        const Expr* fact {with_forall ? nonneg_g(k, g, bad_count ? "4" : "3")
                                      : k.le(k.b.apply(sum, {ga(k, g), N(k, bad_count ? "4" : "3")}, "sum_to(g, n)", k.at()), k.num(k.i64, "6442450941"))};
        const Stmt* lp {k.whl(k.lt(k.v(i), N(k, "3")), {k.cl(k.range(k.v(i), k.i32, "0", "3")), k.cl(fact)}, {k.cl(k.sub(N(k, "3"), k.v(i)))},
                              k.blkc({inc(k, i)}))};
        m->body = k.blkc({k.decl(i, N(k, "0")), lp, k.ret(N(k, "0"))});
        k.done(m);
        k.prog.set_entry(m);
        return lp;
    };
    lc("garr_forall_invariant", "forall nonneg(g, 3) in the invariant of a loop in main (the array is a place of the loop, the values are kept): verifies", [=](Kit& k) { inv_main(k, true, false); });
    lc("garr_pure_invariant", "sum_to(g, 3) <= ... in a loop invariant over a global array: the rows, then the fact, verify", [=](Kit& k) { inv_main(k, false, false); });

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
    lc("alit_main", "(int[]){1, 2, 3}: a hidden array local, three element stores in the expression, the decay in p; p[1] == 2",
       [=](Kit& k) { lit_main(k, 3U, {"1", "2", "3"}, "1", "2", false); });
    lc("alit_main_wrong", "the same, p[1] == 3 fails (check)",
       [=](Kit& k) { k.fail("check", lit_main(k, 3U, {"1", "2", "3"}, "1", "3", false)->loc); });
    lc("alit_zero_rest", "(int[4]){1}: the list leaves three elements out: zero_arr at the declaration, then the store of element 0; p[2] == 0",
       [=](Kit& k) { lit_main(k, 4U, {"1"}, "2", "0", true); });
    lc("alit_outside", "p[3] of a three-element literal: the bound row of the element read through p",
       [=](Kit& k) {
           Function* m {k.fn("main", k.i32)};
           Var* x {lit(k, m, 3U, false)};
           Var* p {k.local(m, "p", k.ptr(k.i32))};
           const Expr* r {k.deref(k.b.elem_addr(k.b.text(k.v(p), "p"), N(k, "3"), k.at()))};
           m->body = k.blkc({k.decl(x, nullptr), k.decl(p, expr_of(k, x, {"1", "2", "3"})), k.chk(k.eq_c(r, k.i32, "0")), k.ret(N(k, "0"))});
           k.done(m);
           k.fail("out-of-bounds", r->loc);
       });
    lc("alit_unwritten", "a literal list that writes fewer elements than the array and is not zero-filled: the element read has no value (uninitialized)",
       [=](Kit& k) {
           Function* m {k.fn("main", k.i32)};
           Var* x {lit(k, m, 3U, false)};
           Var* p {k.local(m, "p", k.ptr(k.i32))};
           const Expr* r {k.deref(k.b.elem_addr(k.b.text(k.v(p), "p"), N(k, "2"), k.at()))};
           m->body = k.blkc({k.decl(x, nullptr), k.decl(p, expr_of(k, x, {"1"})), k.chk(k.eq_c(r, k.i32, "0")), k.ret(N(k, "0"))});
           k.done(m);
           k.fail("uninitialized", r->loc);
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

} // namespace

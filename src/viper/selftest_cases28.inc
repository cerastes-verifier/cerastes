// (included by selftest.cpp) Arrays, M4.5e1a Parts A and B (decision 0059 items 2 and 4): the type `Array` and the node `ElemAddr` exist in VIR, and check_program
// accepts only an automatic array local (a cell of a scalar or accepted-pointer element) and `ElemAddr` over its address; every other position that could hold
// one is a check error naming it.  These are the rejection cases of that fail-closed state; the lowering cases of the accepted shapes are in the later files.
namespace {

void define_arrays_rejected_until_part_b()
{
    auto N = [](Kit& k, const char* v)
    {
        return k.num(k.i32, v);
    };
    auto PI = [](Kit& k)
    {
        return k.ptr(k.i32);
    };
    auto A3 = [](Kit& k)
    {
        return k.prog.array_of(k.i32, 3U);
    };
    // `&p[1]` of the pointer parameter p of `f`
    auto ea = [=](Kit& k, Function* f)
    {
        return k.b.elem_addr(k.b.text(k.pv(f, 0), "p"), N(k, "1"), k.at());
    };

    // ---- the node `ElemAddr`, in every position ----
    rc("arr_rej_elem_addr_as_value", "of anything but a local array",
       [=](Kit& k)
       {
           // int *q = &p[1];
           Function* f {k.fn("f", k.vd, {{"p", PI(k)}})};
           Var* q {k.local(f, "q", PI(k))};
           f->body = k.blk({k.decl(q, ea(k, f))});
           k.done(f);
       });
    rc("arr_rej_elem_addr_read", "of anything but a local array",
       [=](Kit& k)
       {
           // return p[1];
           Function* f {k.fn("f", k.i32, {{"p", PI(k)}})};
           f->body = k.blk({k.ret(k.deref(ea(k, f)))});
           k.done(f);
       });
    rc("arr_rej_elem_addr_write", "of anything but a local array",
       [=](Kit& k)
       {
           // p[1] = 5;
           Function* f {k.fn("f", k.vd, {{"p", PI(k)}})};
           f->body = k.blk({k.store(ea(k, f), N(k, "5"))});
           k.done(f);
       });
    rc("arr_rej_elem_addr_compound_write", "of anything but a local array",
       [=](Kit& k)
       {
           // p[1] += 5;
           Function* f {k.fn("f", k.vd, {{"p", PI(k)}})};
           f->body = k.blk({k.cstore(ea(k, f), BinaryOp::Add, N(k, "5"))});
           k.done(f);
       });
    rc("arr_rej_elem_addr_write_as_value", "of anything but a local array",
       [=](Kit& k)
       {
           // return p[1] = 5;
           Function* f {k.fn("f", k.i32, {{"p", PI(k)}})};
           f->body = k.blk({k.ret(k.b.store_expr(ea(k, f), N(k, "5"), k.at()))});
           k.done(f);
       });
    rc("arr_rej_elem_addr_in_requires", "the address of an array element in a contract",
       [=](Kit& k)
       {
           // requires(p[1] == 0)
           Function* f {k.fn("f", k.vd, {{"p", PI(k)}})};
           k.req(f, k.eq(k.deref(k.b.elem_addr(k.F(f, 0), N(k, "1"), k.at())), N(k, "0")));
           f->body = k.blk({});
           k.done(f);
       });
    rc("arr_rej_elem_addr_in_ensures", "the address of an array element in a contract",
       [=](Kit& k)
       {
           // ensures(p[1] == 0)
           Function* f {k.fn("f", k.vd, {{"p", PI(k)}})};
           k.ens(f, k.eq(k.deref(k.b.elem_addr(k.F(f, 0), N(k, "1"), k.at())), N(k, "0")));
           f->body = k.blk({});
           k.done(f);
       });
    rc("arr_rej_elem_addr_in_check", "of anything but a local array",
       [=](Kit& k)
       {
           // check(p[1] == 0)
           Function* f {k.fn("f", k.vd, {{"p", PI(k)}})};
           f->body = k.blk({k.chk(k.eq(k.deref(ea(k, f)), N(k, "0")))});
           k.done(f);
       });
    rc("arr_rej_elem_addr_in_condition", "of anything but a local array",
       [=](Kit& k)
       {
           // if (p[1] == 0) { }
           Function* f {k.fn("f", k.vd, {{"p", PI(k)}})};
           f->body = k.blk({k.iff(k.eq(k.deref(ea(k, f)), N(k, "0")), k.blk({}))});
           k.done(f);
       });
    rc("arr_rej_elem_addr_in_loop_invariant", "of anything but a local array",
       [=](Kit& k)
       {
           // while (true) invariant(p[1] == 0) decreases(0) { }
           Function* f {k.fn("f", k.vd, {{"p", PI(k)}})};
           f->body = k.blk({k.whl(k.yes(), {k.cl(k.eq(k.deref(ea(k, f)), N(k, "0")))}, {k.cl(N(k, "0"))}, k.blk({}))});
           k.done(f);
       });
    rc("arr_rej_elem_addr_in_atom", "the address of an array element in a contract",
       [=](Kit& k)
       {
           // requires(owns(p[1]))
           Function* f {k.fn("f", k.vd, {{"p", PI(k)}})};
           k.req(f, k.b.atom(AtomKind::Owns, k.b.elem_addr(k.F(f, 0), N(k, "1"), k.at()), 0, k.at()));
           f->body = k.blk({});
           k.done(f);
       });
    rc("arr_rej_elem_addr_ill_typed_base", "of anything but a local array",
       [=](Kit& k)
       {
           // a base that is no pointer is built as it is and refused
           Function* f {k.fn("f", k.vd)};
           Var* q {k.local(f, "q", k.i32)};
           f->body = k.blk({k.decl(q, N(k, "0")), k.es(k.deref(k.b.elem_addr(N(k, "0"), N(k, "1"), k.at())))});
           k.done(f);
       });

    // ---- the type `Array`, in every position ----
    rc("arr_rej_array_local_not_a_cell", "internal: the array local",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.vd)};
           Var* a {k.local(f, "a", A3(k))};
           f->body = k.blk({k.decl(a, nullptr)});
           k.done(f);
       });
    rc("arr_rej_array_parameter", "is not a bool, an integer type or a pointer to one",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.vd, {{"a", A3(k)}})};
           f->body = k.blk({});
           k.done(f);
       });
    rc("arr_rej_array_result", "is not void, bool, an integer type or a pointer to one",
       [=](Kit& k)
       {
           Function* f {k.fn("f", A3(k))};
           k.trusted(f);
           (void)f;
       });
    rc("arr_rej_pointer_to_array_local", "a pointer to int[3]",
       [=](Kit& k)
       {
           // int (*q)[3];
           Function* f {k.fn("f", k.vd)};
           Var* q {k.local(f, "q", k.ptr(A3(k)))};
           f->body = k.blk({k.decl(q, nullptr)});
           k.done(f);
       });
    rc("arr_rej_pointer_to_array_parameter", "a pointer to int[3]",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.vd, {{"q", k.ptr(A3(k))}})};
           f->body = k.blk({});
           k.done(f);
       });
    rc("arr_rej_array_value", "an array has no value",
       [=](Kit& k)
       {
           // a read of a whole array through a pointer parameter: `*q` of an `int (*q)[3]`
           Function* f {k.fn("f", k.vd, {{"q", k.ptr(A3(k))}})};
           f->body = k.blk({k.es(k.deref(k.pv(f, 0)))});
           k.done(f);
       });
    rc("arr_rej_array_struct_member", "is not a bool, an integer, a pointer or a struct",
       [=](Kit& k)
       {
           // struct S { int a[3]; };
           k.strukt("S", {{"a", A3(k)}});
       });
    rc("arr_rej_array_global", "is not a bool, an integer or a pointer to one",
       [=](Kit& k)
       {
           // int g[3];
           k.prog.add_global("g", A3(k), k.at(), nullptr);
       });
    rc("arr_rej_array_zero_length", "of no elements",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.vd)};
           Var* a {k.cell(f, "a", k.prog.array_of(k.i32, 0U))};
           f->body = k.blkc({k.decl(a, nullptr)});
           k.done(f);
       });
    rc("arr_rej_array_of_void_pointers", "is an array of",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.vd)};
           Var* a {k.cell(f, "a", k.prog.array_of(k.ptr(k.vd), 2U))};
           f->body = k.blkc({k.decl(a, nullptr)});
           k.done(f);
       });
    rc("arr_rej_array_with_an_initializer_value", "declared with an initializer value",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.vd)};
           Var* a {k.cell(f, "a", A3(k))};
           f->body = k.blkc({k.decl(a, N(k, "0"))});
           k.done(f);
       });
    rc("arr_rej_array_owning", "is marked owning",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.vd)};
           Var* a {k.cell(f, "a", k.prog.array_of(PI(k), 2U))};
           a->owning = true;
           f->body = k.blkc({k.decl(a, nullptr)});
           k.done(f);
       });
    rc("arr_rej_zero_filled_scalar", "is marked zero-filled but is not an array",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.vd)};
           Var* x {k.local(f, "x", k.i32)};
           x->zero_filled = true;
           f->body = k.blk({k.decl(x, nullptr)});
           k.done(f);
       });
    rc("arr_rej_array_address_as_a_value", "outside an element address",
       [=](Kit& k)
       {
           // int *q = a; (the decay, Part D)
           Function* f {k.fn("f", k.vd)};
           Var* a {k.cell(f, "a", A3(k))};
           Var* q {k.local(f, "q", PI(k))};
           f->body = k.blkc({k.decl(a, nullptr), k.decl(q, k.addr(a))});
           k.done(f);
       });
    rc("arr_rej_element_index_not_an_integer", "not an integer",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.vd)};
           Var* a {k.cell(f, "a", A3(k))};
           f->body = k.blkc({k.decl(a, nullptr), k.es(k.deref(k.b.elem_addr(k.addr(a), k.yes(), k.at())))});
           k.done(f);
       });
    rc("arr_rej_element_index_enum", "not an integer",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.vd)};
           Var* a {k.cell(f, "a", A3(k))};
           const Type en {k.en("E", 32, true, "0", "2")};
           f->body = k.blkc({k.decl(a, nullptr), k.es(k.deref(k.b.elem_addr(k.addr(a), k.num(en, "1"), k.at())))});
           k.done(f);
       });
    rc("arr_rej_array_of_arrays_local", "is an array",
       [=](Kit& k)
       {
           // int a[2][3]: built as is, refused
           Function* f {k.fn("f", k.vd)};
           Var* a {k.cell(f, "a", k.prog.array_of(A3(k), 2U))};
           f->body = k.blkc({k.decl(a, nullptr)});
           k.done(f);
       });
}

} // namespace

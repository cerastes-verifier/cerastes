# VeriFast template tests (unmodified originals)

Source: `mborland/verifast`, branch `generic2`, commit
`0fb63506a51cae559ae92348eb54ba7f7be0bf6d`, directory `tests/cxx/templates/`.
The six files here are byte-identical copies (checked with `cmp`):

| File | `testsuite.mysh` line (in `cd templates`) |
|---|---|
| `generic_body.cpp` | `verifast_both -c generic_body.cpp` |
| `function_template_general_contract.cpp` | `verifast_both -c function_template_general_contract.cpp` |
| `integral.cpp` | `verifast_both -c integral.cpp` |
| `integral_wrong_spec.cpp` | `verifast_both -c -allow_should_fail integral_wrong_spec.cpp` |
| `integral_per_instantiation.cpp` | `verifast -c integral_per_instantiation.cpp` |
| `integral_fallback.cpp` | `verifast_both -c integral_fallback.cpp` |

`verifast_both` runs `verifast` with the default prover and with
`-prover redux` (see the `let verifast_both` at the top of `testsuite.mysh`).
`-c` checks the file without running `main` as a program entry; 
`-allow_should_fail` makes VeriFast accept `//~ should_fail` markers.

`//~ should_fail` marks a line where verification is expected to fail:
VeriFast reports the failure there and, with `-allow_should_fail`, counts the
test as passing if the failure occurs at exactly that line and all other
lines verify. Only `integral_wrong_spec.cpp` uses it.

Annotation-free copies for IL inspection are in `tests/il/verifast-plain/`.
The `//@` annotations become `[[cerastes::]]` attributes in M4.

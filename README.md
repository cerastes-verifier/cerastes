# cerastes

A deductive verifier for C and C++ that verifies template code generically.
Contracts are written as `[[cerastes::...]]` attributes. The EDG front end
parses them, and the program is encoded into Viper and verified with Silicon
and Z3. Errors are reported against the C/C++ source.

Status: toolchain and test harness only (milestone M0). See `plan.md` for the
roadmap and `docs/status.md` for where things stand.

## Getting started (macOS on Apple silicon)

```sh
git clone https://github.com/cerastes-verifier/cerastes
cd cerastes
scripts/bootstrap.sh   # Homebrew prerequisites, submodules, EDG + Silicon builds, pinned Z3
scripts/check.sh       # build, lint, harness self-test, tests
```

Bootstrap needs Homebrew and the Xcode Command Line Tools. It is idempotent.
The tool versions it pins are listed in `docs/decisions/0001-toolchain-pins.md`.

## License

Apache License v2.0 with LLVM Exceptions; see `LICENSE.txt`. The submodules
under `third_party/` carry their own licenses.

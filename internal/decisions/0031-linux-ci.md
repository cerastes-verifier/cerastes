# 0031 — Linux CI: `check.sh` on GitHub's `ubuntu-26.04` x86-64 runner beside `xcode-27` (S5)

- **Status:** accepted for S5. Owner decisions: a Linux job is the first sub-phase after M4.5a ("the first sub phase
  should be adding Linux support to the CI. If MSVC is easy add it, if not defer", 2026-10-27; MSVC and arm64 Linux are
  M8 entries); the runner is `ubuntu-26.04` ("ubuntu-26.04 is available with github actions. If this doesn't work, then
  use the newest toolchains available in that container", handoff §8 question 57, 2026-10-28); the box may be used for
  any testing (question 58).
- **Milestone:** S5 (2026-10-28), after M4.5a, before M4.5b1.
- **Amends:** 0001 (the Linux pins), 0015 item 6 (glibc's `assert`), 0016 (item 13's fallback is built; items 1, 7,
  10 and 14 apply to both jobs).

## Context

Since S2 every check has run on one platform: the owner's Mac and GitHub's `xcode-27` image with the same Command Line
Tools (0016). Decision 0016 item 13 recorded what an x86-64 Linux job would need: glibc's `assert` in the encoder, a
driver without the clang-only `__builtin_available` and the macOS `posix_spawn_file_actions_addchdir`, a Linux
`bootstrap.sh`, the Linux value of `GNU_VECTOR_TYPES_ALLOWED`, and every expectation that depends on the system headers
re-established. M4.5a's survey added the C++ standard library (EDG in Clang emulation over libstdc++ is a risk;
libc++ is what macOS has) and P55's glibc half (vlibc's shim against glibc's `<stdlib.h>`).

Facts checked while planning (2026-10-28):
- GitHub's `ubuntu-26.04` image (`20261023.163.3`, Ubuntu 26.04.1 LTS, x86-64): clang 20.1.8, 21.1.8 and 22.1.2; GCC
  13.4, 14.3 and 15.2 (no 16); JDKs 11, 17 (`JAVA_HOME_17_X64`, 17.0.20+1), 21 and 25 (the default); CMake 4.4.4,
  Ninja 1.13.2, Python 3.14, Node 24; ShellCheck 0.11.0; sbt is not listed.
- Ubuntu 26.04's archive (`resolute`): `gcc-16`/`g++-16` `16-20260322-1ubuntu1`, a trunk snapshot that calls itself
  `16.0.1 20260322 (experimental)`; GCC 16.2.0 is packaged only from 26.10 (`stonking`, `tenacious`). `clang-21`,
  `libc++-21-dev`, `libc++abi-21-dev` `1:21.1.8-6ubuntu1` (banner `Ubuntu clang version 21.1.8 (6ubuntu1)`); glibc
  2.43 (`libc6-dev` `2.43-2ubuntu2.4`).
- The toolchain PPA (`ubuntu-toolchain-r/test`) has no GCC 16 for `resolute`.
- The goldens name libc++'s inline namespace (`std::__1::integral`, 10 goldens), so libstdc++ would change them.
- The box (`192.168.1.167`) did not answer during the session (no ping, SSH timed out), so the rehearsal ran in an
  `ubuntu:26.04` amd64 container on the owner's Mac (Docker Desktop, Rosetta, 4 CPUs, 8 GB).

## Decision

1. **The job.** `.github/workflows/ci.yml` gets a second job, `check-linux`, after `check`: `runs-on: ubuntu-26.04`,
   the same triggers, permissions, concurrency, checkout, Python and Node steps, Silicon cache (its key carries
   `runner.os`, so the two jobs never share a jar), `timeout-minutes: 220` and the same `CERASTES_CHECK_BASE`
   expression, so both tiers of 0016 item 14 run on both platforms. `CERASTES_JOBS: 4` (GitHub's Linux runner for a
   public repository has 4 cores and 16 GB). The setup step runs `scripts/ci-linux.sh`, then `scripts/check.sh` runs
   unchanged; the evidence is uploaded as `ci-linux-<run>-<attempt>`. No `needs:`: the jobs run in parallel and each
   fails on its own. Which jobs are required on `main` is the owner's branch-protection setting.
2. **The toolchain** (`scripts/ci-linux.sh`, the counterpart of `scripts/ci.sh`). It asserts Ubuntu 26.04 on x86-64, at
   least 6 GB free, none of the build-changing variables of 0016 item 7 (plus `LD_LIBRARY_PATH` and `LD_PRELOAD`, and
   without the macOS-only ones), and `JAVA_HOME_17_X64`; installs from Ubuntu's archive, with `apt-get install
   --no-install-recommends`, `gcc-16`, `g++-16`, `clang-21`, `libc++-21-dev`, `libc++abi-21-dev` and `shellcheck`
   (what the image already has stays as it is); and asserts exactly: the three LLVM packages `1:21.1.8-6ubuntu1`, the
   two GCC packages `16-20260322-1ubuntu1`, `gcc-16 -dumpfullversion` 16.0.1, the clang banner, glibc 2.43,
   `/usr/bin/clang-21` and `/usr/bin/clang++-21`, `gcc-16` as the highest `gcc-NN` on `PATH`, and after bootstrap
   `build/env.sh`'s compilers, `EDG_CLANG_VER_SCRAPE` 210108, and C++ include directories that name libc++ 21 and no
   libstdc++. The toolchain report adds the `libc6-dev` and `linux-libc-dev` versions (recorded, not asserted: the
   image's security updates move them; a change shows in the report and, if it changes a result, as drift).
   - **GCC 16 is the snapshot** (owner, question 57: "the newest toolchains available"): 26.04 has no GCC 16.2.0. The
     native-ok runs on Linux compile with it; a warning that it gives and GCC 16.2.0 does not, or the reverse, is
     drift (0016 item 10) and goes to the owner.
   - **JDK 17** is the image's (`JAVA_HOME_17_X64`, Temurin 17.0.20+1); on a machine without that variable,
     bootstrap uses `/usr/lib/jvm/java-17-openjdk-amd64` (Ubuntu's `openjdk-17-jdk-headless`). Silicon's stamp records
     the JDK's runtime line instead of the Homebrew formula.
   - **sbt**: the 2.0.10 launcher, the version Homebrew gives the owner's Mac (it starts Silicon's pinned sbt 1.6.2),
     downloaded by bootstrap into `build/tools` from sbt's GitHub release (`sbt-2.0.10.zip`, SHA-256
     `6ff28e1d…90cd7`, the digest GitHub publishes for the asset). The zip, not the `.tgz`: Z3 already needs `unzip`,
     and GNU tar 1.35 failed in the rehearsal container (`Function not implemented` under Rosetta).
   - **Z3** 4.16.0 `z3-4.16.0-x64-glibc-2.39.zip`, SHA-256 `7288c49a…3418e` (GitHub's digest; the same source gives the
     pinned macOS asset's digest exactly).
   - Python and Node come from the same `setup-python` and `setup-node` steps as on macOS; CMake and Ninja are the
     image's, recorded in the log, not pinned (as in 0001).
3. **EDG on Linux.** `bootstrap.sh` uses EDG's `linux-clang-debug` preset (Clang emulation, as 0002 chose on macOS;
   target `linux-x86_64`, EDG_BASE `bases/docker/dev-env/clang`, EDG's default for that preset) and passes
   `/usr/bin/clang-21` and `/usr/bin/clang++-21` as the compilers (the preset names plain `clang`, which the image may
   point at another version). Two stock settings differ from the preset's defaults: `-DEDG_CPFE_USE_ASAN=OFF` (EDG
   turns AddressSanitizer on for Linux debug builds only; macOS's stock build has none) and `-DEDG_BUILD_LIB=OFF`, no
   EDG C++ runtime library. Nothing in cerastes links that library (`cpfe` and `cerastes-fe` never generate code
   that needs it); the Linux preset builds it for eight targets through `eccp`, which runs a plain `clang` (its
   EDG_BASE's `edg_eccp_config`), and clang 21 rejects its `bf16` literals (`c99_complex.c`, "invalid suffix 'bf16'
   on floating constant"). cerastes's own CMake build uses clang++-21 too.
   - **The C++ library is libc++ 21** (`-stdlib=libc++`): the scrape runs clang++-21 through a two-line wrapper,
     `build/tools/clang++-libc++`, given to EDG's `edg-scrape-compiler` as `--compiler-path`, so the scrape itself is
     EDG's, unchanged. The C headers are glibc's. libc++ is what macOS has, and keeps the goldens' `std::__1`;
     libstdc++ (clang's default on Linux) would rename them and is not needed by any test.
   - **The emulated clang is 21.1.8** (`EDG_CLANG_VER_SCRAPE` 210108; 210000 on macOS, Apple clang 21.0.0). The
     target facts that the encoder reads (0010 item 1: sizes, the signedness of plain `char`, `wchar_t`) are the same
     on x86-64 Linux as on macOS on Apple silicon for every type cerastes accepts: `int` 32 bits, `long` and pointers 64,
     plain `char` signed, `wchar_t` a signed 32-bit `int`. (`long double` differs; floating point is rejected.)
4. **The driver and the link.** `src/driver/silicon.cpp` keeps the macOS code under `__APPLE__` and calls glibc's
   `posix_spawn_file_actions_addchdir_np` (glibc 2.29 and later) elsewhere; nothing else in `src/` is
   platform-specific. `src/driver/CMakeLists.txt` links `cerastes_backend` and `edg_fe` as a
   `$<LINK_GROUP:RESCAN,…>` on platforms other than Apple: EDG's `cfe_main` calls `back_end()`, which calls back
   into EDG, and GNU ld reads each static archive once, left to right (the first Linux link failed with "undefined
   reference to `back_end()`"); macOS's linker resolves the cycle itself.
5. **glibc's `assert`** (amends 0015 item 6). Besides the SDK's expansion, the encoder recognises exactly glibc 2.43's
   two expansions that strict C23 and C++20 select (`__STRICT_ANSI__` is defined in both, so the GNU
   statement-expression form never appears):
   - C23 (C23 makes `assert` variadic): `(((void) sizeof (__assert_single_arg (e)), e) ? (void) (0) : __assert_fail
     (…))`; the left operand of the comma must be a `void` cast of a folded integer constant **whose original
     expression is the `sizeof`** (never evaluated). *Audit (S5):* the first version accepted any folded constant
     there, so `((void)(INT_MAX + 1), x >= 0) ? (void)0 : __assert_fail(…)` would have verified with its overflow
     unencoded (EDG's #61 is suppressed in verify mode); fixed before commit;
   - C++: `(static_cast<bool> (e) ? void (0) : __assert_fail (…))`; a conversion to `bool` on the condition is looked
     through (EDG leaves it out when `e` is a `bool`), which does not change the truth value tested.
   `__assert_fail` must be what `__assert_rtn` must be on macOS: declared in a system file, external, `void`, does not
   return. Everything else of 0015 item 6 is unchanged: `e` is code, checked as `check` is, a side effect in it is
   rejected, its arguments are never encoded. The rule is on the callee's declaration, not on where the expansion is
   written: on Linux, with `<assert.h>` included, the exact expansion written by hand (or by a user macro) is an
   assertion, as 0015 item 6 says of a user macro and as the SDK's shape is on macOS; a `__assert_fail` declared in a
   user file is not a system function and the call stays `call_without_contract`. macOS has no system
   `__assert_fail` (the audit checked the SDK and the Command Line Tools), so there the glibc shape never matches.
   - **`.vpr` that may differ between the platforms** (audit): when `e` is already `bool`, C++'s `assert((bool)x)` loses
     the user's cast on Linux (EDG leaves glibc's `static_cast<bool>` out because `e` is already a `bool`, and the
     matcher then looks through the user's `(bool)`), so the condition is `x` instead of `(bool)x`: the same truth value and the same checks, different text. `assert(E::b)` with a scoped enumeration
     compiles only under glibc's `static_cast<bool>` (it is an error on macOS). No test has either shape today.
6. **Same outcomes, or stop** (handoff §4.4). Every test and doc example must give the same outcome on both platforms;
   no test gets a per-platform expectation without an owner decision. The rehearsal compares:
   - every result of `run_tests.py` (the outcome lines of the harness, both modes, native-ok included), and
   - **every emitted `.vpr` file byte for byte** (`build/vpr` of the Linux rehearsal against the Mac's, for the same
     commit): with the same Silicon jar, Z3 version and `.vpr`, Silicon's verdict cannot depend on the platform except
     through timing, which 0016 item 10 already treats as drift.
   - **The result** (2026-10-28, the PR tier against `72e30b3` in the container, against the Mac's full run of the
     same tree): every outcome agreed except 12 runs of 8 tests that name what only the macOS SDK declares
     (`__assert_rtn`; `idtype_t` and `P_ALL`, which glibc hides in strict C23; `sys_nerr`, gone since glibc 2.32;
     `abs`, which glibc declares `noexcept` in C++; `quick_exit`, whose glibc declaration carries `__nothrow__`, P55's
     glibc half, M4.5d1). **The owner chose a platform tag** (2026-10-28): `; platform: macos` on the test's `mode:` or
     `c-only:` line (no line moves, no `expect:` line changes); the harness runs such a test only there and reports a
     named skip elsewhere. glibc twins (`platform: linux`) for the assert near misses and P55 come later.
     Of the 653 `.vpr` and map files both platforms produced, 17 differ, all benign: `MANIFEST`; two `.vpr` whose
     trusted libc methods take their parameter names from the system declaration (`exit(int __status)` on glibc,
     `exit(int)` on macOS: `p___status__0` against `p_arg__0`); and position maps that name the system header of an
     internal check (`/usr/include/stdlib.h:752` against the SDK's `_abort.h:33`).
7. **The stock configuration differs; cerastes's does not.** cerastes's EDG build takes EDG's generated
   `cmake_defines.h` from the stock build (0002, `USE_CMAKE_DEFINES`), and the Linux one differs from macOS's in 72
   macros that are not per target: EDG's own code-generation and feature switches (`FIXED_POINT_ALLOWED`,
   `EMBEDDED_C_ALLOWED`, `RISCV_VECTOR_BUILTINS_ENABLED`, `COROUTINE_ENABLING_POSSIBLE`, …), target facts that
   cerastes rejects anyway (`long double` 80-bit, 16 bytes; `wint_t`; `jmp_buf`), and the host representation of
   EDG's integer values (`INTEGER_VALUE_REPR_IS_A_HOST_INTEGER` 0 on Linux, a host `__int128` on macOS). The back end
   reads constants only through EDG's `decimal_str_for_integer_constant`, so the representation does not reach it;
   every macro `check_edg_config.sh` asserted before S5 (`GNU_VECTOR_TYPES_ALLOWED` 1 included) has the same value on
   both platforms.
   - **Five extensions are turned off in `edg_config`** (`cmake/EdgFrontEnd.cmake`), at macOS's stock values:
     `EMBEDDED_C_ALLOWED`, `FIXED_POINT_ALLOWED`, `NAMED_ADDRESS_SPACES_ALLOWED`, `NAMED_REGISTERS_ALLOWED` (TR 18037)
     and `DEFAULT_PASS_STDARG_REFERENCES_TO_GENERATED_CODE`, all 0. The Linux stock configuration has them 1, which
     compiled seven more EDG options into the front end (`fixed_point`, `named_address_spaces`, `named_registers`,
     `no_stdarg_builtin` and their negations), so `check_edg_options.sh` failed against `src/driver/edg_options.inc`,
     and C would have accepted `_Accum` and named address spaces on Linux only. With them off both front ends compile
     the same 386 options and accept the same extensions. macOS's build is unchanged (its `--dump_configuration`
     differs only in the build date). `edg_il.h` and `check_edg_config.sh` assert the five.
   - **What still differs by target** (the front ends' `--dump_configuration`, 99 macros): the ABI and type facts of
     x86-64 against arm64 (`long double`, `jmp_buf`, `wint_t`, the Itanium ABI variants), the host representation of
     values, and two GNU extensions of the x86 target: `GNU_X86_ATTRIBUTES_ALLOWED` and
     `THREAD_LOCAL_STORAGE_SPECIFIER_ALLOWED` (`__thread`), on for Linux only. No test uses either.
8. **The harness.** A test's `mode:` or `c-only:` line may end with `; platform: macos|linux` (item 6). `run_tests.py`'s native clang is labelled `clang` on Linux (`CLANG_FAMILY`; its banner must say
   `clang version`) and `Apple clang` on macOS as before; the self-tests take the label from the harness. A GCC is
   recognised (bootstrap's `is_gcc`, the harness's `banner_problem`) by `GCC` on the first line of `--version`, as
   Homebrew's says, or by the FSF's copyright on the second, as Ubuntu's `gcc-16 (Ubuntu 16-20260322-1ubuntu1)
   16.0.1` needs (the first rehearsal found no GCC: `ci-linux.sh`'s `CERASTES_GCC_CXX` assertion stopped it), and
   never when the first line says clang.
   `CiGuardTests` pins both jobs of `ci.yml` (a spec per job: runner, environment, setup script; the jobs exactly
   `check` then `check-linux`) and the whole active text of `scripts/ci-linux.sh` with the same rules as `ci.sh`
   (`ci_sh_problems` takes a spec), with mutation tests for each.
9. **Predefined macros that glibc undefines and defines again** (amends 0008 item 2). glibc 2.43's `<stdint.h>`, which
   libc++'s `<concepts>` and `<cstdint>` include, has `#undef __INT64_C`, `#undef __UINT64_C` and then `#define
   __INT64_C(c) c ## L` and `#define __UINT64_C(c) c ## UL`: the definitions clang predefines on x86-64 Linux, in
   other whitespace. 0008's R4c rejected any `#define` or `#undef` of a predefined macro outside `<cerastes.h>`, so on
   Linux every test that includes those headers failed (the ten walker goldens over `<concepts>` among them; none of the
   macOS headers that the tests include touches a predefined macro). Now an `#undef` of a predefined macro, and the `#define` that follows it, are no
   change when all of these hold (anything else stays `annotation_macro_changed`):
   - both are **in a system file**: EDG's flag on the directive's logical file record (set by a `--sys_include`
     directory, by `#pragma GCC system_header` from its line on, by a GNU linemarker with flag 3, and for any file
     that a system header includes), which is what walk mode calls a system file everywhere; in
     verify mode also 0011 item 4's rule **by path**, under which `/usr/local/include`, the `-I` directories and a
     user file that a system header includes are user files (audit: EDG flags any file a system header includes, and
     the SDK's `<Availability.h>` includes `<AvailabilityInternalPrivate.h>` when one exists on the path);
   - the `#define` is the next directive for that name after the `#undef`, **in the same inclusion** (the same file
     record), and **no file record starts strictly between them**, so no file with a line in it is included there (audit: an `#undef` in one inclusion and the `#define` in a
     later one left user code between them running with the macro undefined);
   - the `#define`'s text is **exactly** the last predefinition's (`strcmp`, and the same object-like flag). EDG
     normalizes the text it records (`c##L` and `c ## L` are both `c ## L`, `( a , b )` is `(a,b)`, string literals
     are kept as written), so glibc's line equals clang's predefinition; a string with other spacing does not (audit:
     the first version compared tokens split at whitespace, and accepted `"21.0"` for `"21.0 "`).
   A `#define` of a predefined name with no such `#undef` before it is reported as before (an identical one is never
   recorded by EDG). Only the system header's own lines between the pair see the macro undefined, nothing outside
   them does, and the macro is the predefinition again after the pair; an annotation on those lines is that header's
   own, trusted like the rest of it. *For M4.5d1:* vlibc's contract headers reach the front end through
   `--sys_include` (record 0030 item 4), so EDG flags them as system; in verify mode the path rule decides, and vlibc's
   library class must count as a verified file for this exemption (or be excluded from it) before vlibc lands. Tests:
   `tests/annotations/macros/system_header_redefinition.cpp` (a golden; its helper header undefines and redefines
   `__CHAR_BIT__`, 8 on both platforms) and `SystemHeaderMacroTests` in `scripts/test_run_tests.py` (11 cases: another
   value, an `#undef` left standing, the `#define` in another file, in a second inclusion, a file included in between,
   a user header, a header before its pragma, a string with other spacing, a function-like predefinition with a
   renamed parameter on Linux; the harness cannot name a diagnostic in a header, so these are self-tests).
10. **What is not done here.** P55's glibc half (vlibc's shim against glibc's `<stdlib.h>`, and
   `reject_quick_exit_user_declared.c`'s glibc twin) waits for M4.5d1, which has the shim; the glibc `assert`
   matcher's near-miss twins (`platform: linux`) and the tighter `sizeof` operand come with it (handoff §8 question
   59). Silicon on Linux for the tests outside the PR tier's sample is first seen on GitHub's runner (a full run by
   dispatch, or the nightly). VeriFast's Linux binary in M5c's Docker step can use this runner. arm64 Linux and MSVC
   stay M8 entries (plan.md).

## Consequences

- Every pull request, push to `main` and nightly run checks cerastes on two platforms, two C libraries (the macOS SDK's
  and glibc) and two builds of libc++.
- A test whose outcome depends on the system headers now has to give the same outcome against both; a new system
  header in a test is checked on both platforms by CI.
- A drift in either image stops the work as in 0016 item 10; the Linux job's expected values change with this record
  and 0001, never alone.

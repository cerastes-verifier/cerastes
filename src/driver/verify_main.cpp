// Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// `cerastes verify <file> [options]` (decision 0011): runs EDG in process with the cerastes configuration, lets
// the back end encode the IL to Viper (src/backend/encode.cpp, called from back_end()), then runs Silicon on
// the result and reports its errors at C/C++ lines.
//
// Exit status: 0 verified; 1 verification errors; 2 the program is not accepted (a front-end error, a cerastes
// rejection, a refused option); 3 a tool failure.
//
// Streams (0011 item 9): the verdict, the verification errors and their notes go to standard output;
// front-end diagnostics go to standard error (EDG prints them itself, and the back end's rejections come
// with it), always with their error numbers; a refused option is one `cerastes: error:` line on standard
// error.  Nothing else is printed.
//
// The command line is an allow-list: no EDG option passes through, so `--diag_suppress`, `--no_code_gen` and the
// like cannot reach the front end.
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

#include "edg_args.h"
#include "pipeline.h"
#include "register_attributes.h"
#include "report.h"
#include "run_state.h"
#include "selftest.h"
#include "silicon.h"
#include "silicon_output.h"
#include "tool_paths.h"
#include "verify_config.h"

// EDG's entry point, renamed from main by EDG_MAIN (host_envir.h).
extern int edg_main(int argc, char** argv);

namespace {

constexpr const char kVersion[] = "cerastes (M3 development build)";

constexpr const char kUsage[] =
    "usage: cerastes verify <file> [options]\n"
    "       cerastes --help | --version\n"
    "\n"
    "Verifies every function of <file> and of the user headers it includes against its contract.\n"
    "\n"
    "options:\n"
    "  -I<dir>, -I <dir>        add an include directory\n"
    "  -D<name>[=<value>]       define a macro (-D__CERASTES__ is refused)\n"
    "  -U<name>                 undefine a macro\n"
    "  --std=c++20|c23          the language (default: c23 for .c files, c++20 otherwise)\n"
    "  --keep-vpr[=<path>]      keep the Viper program: <path> and <path>.map (default <stem>.vpr)\n"
    "  --timeout=<seconds>      wall-clock limit for Silicon (default 300)\n"
    "  --help, --version\n"
    "\n"
    "exit status: 0 verified, 1 verification errors, 2 the program is not accepted, 3 tool failure\n";

struct Args {
  std::string file;
  std::string std_mode;  // "c++20" or "c23"
  std::vector<std::string> includes;
  std::vector<std::string> macro_options;  // "-DNAME=VALUE" / "-UNAME", in order
  bool keep = false;
  std::string keep_path;
  int timeout = 300;
  std::string hook;
};

int refuse(const std::string& message) {
  std::fprintf(stderr, "cerastes: error: %s\n", message.c_str());
  return 2;
}

// drop_assert[=N]: drop the N-th assert (default 0); weaken_invariant: fail the automatic range invariants;
// constant_without_expr: treat a folded constant as having no expression; silicon_timeout: run Silicon with a
// timeout of zero; abort: abort() inside the back end (a front-end crash); internal_error: an EDG internal error raised from the back end
// (both exit status 3); perturb_folded: move every named constant's value and every case value's front-end value to a
// neighbouring value of its type, so the internal guards of decision 0017 fail (exit status 3); perturb_fixed_order: swap the first
// two fixed points of a full expression's call order, so that lowering's fail-closed order check reports its internal error
// (decisions 0021 and 0022 item 12; exit status 3); drop_block_static_assertion: the encoder ignores the static assertions of function
// bodies (neither records nor encodes them), so that the cross-check of decision 0025 item 5 reports them (exit status 2);
// force_base_sets: every full expression takes the base sets (today's, decision 0026 item 2 step 2) instead of the exact ones, so the Viper of a
// program can be compared with the one before exact sets (no change in the verdict, only in how many false alarms remain).
bool is_hook(const std::string& h) {
  if (h == "drop_assert" || h == "weaken_invariant" || h == "constant_without_expr" || h == "perturb_folded" || h == "perturb_fixed_order" || h == "force_base_sets" || h == "drop_block_static_assertion" || h == "silicon_timeout" || h == "abort" || h == "internal_error") return true;
  if (h.rfind("drop_assert=", 0) != 0 || h.size() == 12) return false;
  for (size_t i = 12; i < h.size(); ++i)
    if (h[i] < '0' || h[i] > '9') return false;
  return h.size() < 20;
}

std::vector<std::string> split_colons(const char* s) {
  std::vector<std::string> out;
  std::string cur;
  for (const char* p = s; *p; ++p) {
    if (*p == ':') {
      if (!cur.empty()) out.push_back(cur);
      cur.clear();
    } else {
      cur += *p;
    }
  }
  if (!cur.empty()) out.push_back(cur);
  return out;
}

bool ends_with(const std::string& s, const char* suffix) {
  size_t n = std::strlen(suffix);
  return s.size() >= n && s.compare(s.size() - n, n, suffix) == 0;
}

// Returns 0 to go on, else the exit status (a refusal, or --help).
int parse_args(int argc, char** argv, Args& a) {
  for (int i = 0; i < argc; ++i) {
    const std::string s = argv[i];
    auto value_of = [&](const char* flag, std::string* out) -> int {
      // `-I dir` / `-Idir`: the value is the rest of the word, or the next word.
      if (s.size() > 2) {
        *out = s.substr(2);
        return 0;
      }
      if (i + 1 >= argc) return refuse(std::string("option ") + flag + " needs a value");
      *out = argv[++i];
      return 0;
    };
    if (s == "--help" || s == "-h") {
      std::fputs(kUsage, stdout);
      return -1;
    } else if (s == "--version") {
      std::printf("%s\n", kVersion);
      return -1;
    } else if (s.rfind("--std=", 0) == 0) {
      std::string v = s.substr(6);
      if (v != "c++20" && v != "c23")
        return refuse("--std=" + v + " is not supported: use --std=c++20 or --std=c23 (GNU emulation and C before C23 are refused)");
      a.std_mode = v;
    } else if (s == "--keep-vpr") {
      a.keep = true;
    } else if (s.rfind("--keep-vpr=", 0) == 0) {
      a.keep = true;
      a.keep_path = s.substr(11);
      if (a.keep_path.empty()) return refuse("--keep-vpr= needs a path");
    } else if (s.rfind("--timeout=", 0) == 0) {
      const std::string v = s.substr(10);
      char* end = nullptr;
      long t = std::strtol(v.c_str(), &end, 10);
      if (v.empty() || *end != '\0' || t < 1 || t > 86400) return refuse("--timeout needs a whole number of seconds from 1 to 86400");
      a.timeout = static_cast<int>(t);
    } else if (s.rfind("--cerastes_verify_selftest=", 0) == 0) {
      a.hook = s.substr(27);
      if (!is_hook(a.hook)) return refuse("unknown self-test hook \"" + a.hook + "\"");
    } else if (s.rfind("-I", 0) == 0) {
      std::string dir;
      if (int rc = value_of("-I", &dir)) return rc;
      if (dir.empty()) return refuse("-I needs a directory");
      a.includes.push_back(dir);
    } else if (s.rfind("-D", 0) == 0 || s.rfind("-U", 0) == 0) {
      std::string v;
      if (int rc = value_of(s.rfind("-D", 0) == 0 ? "-D" : "-U", &v)) return rc;
      const char* opt = s[1] == 'D' ? "-D" : "-U";
      if (v.empty()) return refuse(std::string(opt) + " needs a name");
      std::string why = cerastes::args::macro_refusal(v, opt, /*verify_mode=*/true);
      if (!why.empty()) return refuse(why);
      a.macro_options.push_back(std::string(opt) + v);
    } else if (!s.empty() && s[0] == '-') {
      return refuse("option \"" + s + "\" is not supported by \"cerastes verify\" (run `cerastes verify --help`)");
    } else {
      if (!a.file.empty()) return refuse("only one source file can be verified at a time (\"" + a.file + "\" and \"" + s + "\")");
      a.file = s;
    }
  }
  if (a.file.empty()) return refuse("no source file given (usage: cerastes verify <file> [options])");
  if (a.std_mode.empty()) a.std_mode = ends_with(a.file, ".c") ? "c23" : "c++20";
  return 0;
}

bool readable(const char* path, int mode) { return ::access(path, mode) == 0; }

// EDG reads some environment variables (host_envir.c); a stray one would change what it finds or how it prints.
// The data directory is the configured one (0011 item 3), colors are off, and the others are removed.
void scrub_environment() {
  for (const char* name : {"USR_INCLUDE", "EDG_MODULES_PATH", "EDG_DEFAULT_SYSTEM_LOCALE", "LIBPATH", "EDG_COLORS",
                           "GCC_COLORS", "EDG_CPPCLI_PORTABLE_ASSEMBLY_PATH", "EDG_CPPCX_INCLUDE_PATH"})
    ::unsetenv(name);
  ::setenv("EDG_BASE", CERASTES_TOOL_EDG_BASE, 1);
  ::setenv("NOCOLOR", "1", 1);
}

// A front-end abort (EDG's internal-error path ends in abort()) or a crash of ours is a tool failure: exit
// status 3 with one line (decision 0011 item 8).  Only async-signal-safe calls.
// The temporary directory of EDG's diagnostics, for the handlers to remove (plain data, async-signal-safe to use).
char g_diag_file[1024] = "";
char g_diag_dir[1024] = "";
// Copies what EDG wrote to its diagnostics file to standard error (so that what it found before a crash or an
// interruption is not lost), then removes the file and its directory.  Raw system calls on a static buffer only.
void dump_and_remove_diag_files() {
  if (g_diag_file[0] != '\0') {
    int fd = ::open(g_diag_file, O_RDONLY);
    if (fd >= 0) {
      static char buf[4096];
      ssize_t n;
      while ((n = ::read(fd, buf, sizeof buf)) > 0) {
        ssize_t ignored = ::write(2, buf, static_cast<size_t>(n));
        (void)ignored;
      }
      ::close(fd);
    }
    ::unlink(g_diag_file);
  }
  if (g_diag_dir[0] != '\0') ::rmdir(g_diag_dir);
}

extern "C" void crash_handler(int) {
  dump_and_remove_diag_files();
  static const char msg[] = "cerastes: error: internal error in the front end (the program was not checked)\n";
  ssize_t ignored = ::write(2, msg, sizeof msg - 1);
  (void)ignored;
  ::_exit(3);
}

void install_crash_handlers() {
  struct sigaction sa;
  sigemptyset(&sa.sa_mask);
  sa.sa_handler = crash_handler;
  sa.sa_flags = 0;
  for (int sig : {SIGABRT, SIGSEGV, SIGBUS, SIGILL, SIGFPE}) ::sigaction(sig, &sa, nullptr);
}

// SIGINT, SIGTERM and SIGHUP at any time (decision 0011 item 11).  EDG installs its own handlers for them while it
// starts, which exit with status 4 and lose what it wrote; so the signals are blocked in every thread, and a thread of
// ours waits for them: it kills Silicon's process group, shows EDG's diagnostics, removes the temporary files and
// exits with status 3.
void* signal_thread(void*) {
  sigset_t set;
  sigemptyset(&set);
  for (int sig : {SIGINT, SIGTERM, SIGHUP}) sigaddset(&set, sig);
  int sig = 0;
  sigwait(&set, &sig);
  cerastes::silicon_signal_cleanup();
  std::fflush(nullptr);
  dump_and_remove_diag_files();
  static const char msg[] = "cerastes: error: interrupted (the program was not checked)\n";
  ssize_t ignored = ::write(2, msg, sizeof msg - 1);
  (void)ignored;
  ::_exit(3);
  return nullptr;
}

void install_terminate_handlers() {
  sigset_t set;
  sigemptyset(&set);
  for (int sig : {SIGINT, SIGTERM, SIGHUP}) sigaddset(&set, sig);
  pthread_sigmask(SIG_BLOCK, &set, nullptr);
  pthread_t t;
  if (pthread_create(&t, nullptr, signal_thread, nullptr) == 0) pthread_detach(t);
}

std::string slurp(const std::string& path) {
  std::ifstream in(path, std::ios::binary);
  std::ostringstream ss;
  ss << in.rdbuf();
  return ss.str();
}

// EDG's internal errors are reported like diagnostics (`internal error #2656: assertion failed ...`), and exit with
// the same status as a catastrophic error such as a missing source file (EXIT_ON_INTERNAL_ERROR): tell them apart
// by the severity word, at the start of a line or after the position.
bool has_internal_error(const std::string& text) {
  // `"file", line N: internal error #M: ...` or, without a position, `Internal error #M: ...`, at the start of a
  // line: the format of EDG's own diagnostic, which text in the program (`#warning "x: internal error #1"`) cannot
  // produce at the start of a line.
  static const std::regex re(R"(^(\"[^\n]*\", line [0-9]+: )?[Ii]nternal error #[0-9]+: )", std::regex::multiline);
  return std::regex_search(text, re);
}

int verify_command(int argc, char** argv) {
  Args a;
  if (int rc = parse_args(argc, argv, a)) return rc < 0 ? 0 : rc;

  cerastes::SiliconTools tools{CERASTES_TOOL_JAVA, CERASTES_TOOL_SILICON_JAR, CERASTES_TOOL_Z3};
  for (const std::string* t : {&tools.java, &tools.jar, &tools.z3}) {
    if (!readable(t->c_str(), t == &tools.jar ? R_OK : X_OK)) {
      std::fprintf(stderr, "cerastes: error: the tool %s is missing: run scripts/bootstrap.sh\n", t->c_str());
      return 3;
    }
  }

  const bool c_mode = a.std_mode == "c23";
  const std::vector<std::string> sys_includes =
      split_colons(c_mode ? CERASTES_TOOL_C_SYS_INCLUDES : CERASTES_TOOL_CXX_SYS_INCLUDES);

  // The EDG command line: ours entirely (0011 item 2).
  std::vector<std::string> edg = {
      "cerastes",
      "-D__CERASTES__=1",  // before the user's options (0008)
      std::string("--clang_version=") + CERASTES_TOOL_CLANG_VERSION,
  };
  for (const std::string& d : sys_includes) edg.push_back("--sys_include=" + d);
  // include/ holds <cerastes.h>; as a system include directory, diagnostics for macros used in the caller's file
  // still point at the caller's file (as scripts/cerastes-fe.sh does).
  edg.push_back(std::string("--sys_include=") + CERASTES_TOOL_INCLUDE_DIR);
  edg.push_back(c_mode ? "--c23" : "--c++20");
  edg.push_back("--display_error_number");
  // One line per diagnostic whatever the length of the path (EDG would wrap it before the error number), so that a
  // diagnostic can be read by a program (0011 item 9).  No colors.
  edg.push_back("--no_wrap_diagnostics");
  edg.push_back("--no_colors");
  // The warnings the verifier reports itself (0010 item 12): constant-folding warnings, unused variables, etc.
  // #2803 (an unrecognized attribute) is replaced by the error `unrecognized_attribute` (0011 item 7).
  // #546 (a jump bypasses an initialization) is replaced by cerastes's own rejection of every such jump: a switch body
  // declaration with an initializer that a later case label skips, and a goto past a declaration with an initializer, both
  // `unsupported_statement` (0014 items 1 and 7; 0015 item 2; a declaration without one may be jumped past, 0019 item 6).  #1305 (a function declared not to return may return) is replaced by the `noreturn` check at every
  // reachable `return` and closing brace (0015 item 4).  #1901 (C: use of a const variable in a constant expression, which EDG
  // accepts as an extension) is NOT suppressed, and in C mode it is an error: strict ISO C refuses every constant-expression
  // context (a case value, `_Static_assert`, a file-scope initializer, an array bound) that names an object (C23 6.6p8;
  // 0017 item 2).  The encoder also rejects a case value and a constant's initializer that name an object itself
  // (defence in depth: the diagnostic pragmas that could downgrade #1901 are rejected below).  #188 (C: an enumerated type
  // mixed with another type, as in `enum color c = 2;` or `case 1:` over an enumeration) is replaced by the `conversion`
  // check of every conversion into an enumeration (decision 0018 item 5; P27).
  edg.push_back("--diag_suppress=61,62,63,39,179,68,69,188,546,940,1305,2803");
  // #549 (a variable is used before its value is set) goes by the order of the source, not by paths: it misses the one-armed
  // `if (c) x = 1; return x;` and fires on a dead read, so it is no evidence either way.  cerastes checks every read of a local
  // declared without an initializer itself, on each feasible path (`uninitialized`, decision 0019 items 3 and 7).  The option
  // turns the check off for good: a diagnostic pragma cannot turn it back on, unlike `--diag_suppress=549` (P29).
  edg.push_back("--no_use_before_set_warnings");
  // C: #257 (a `const` local without an initializer, a warning in C and an error in C++) is replaced by cerastes's own
  // rejection of every such local (decision 0019 item 1; P28).  At file scope C gives no #257 (0017 handles `const int z;`).
  if (c_mode) edg.push_back("--diag_suppress=257");
  if (c_mode) edg.push_back("--diag_error=1901");
  // A pragma EDG does not understand (#161 unrecognized #pragma, #1675 unrecognized GCC pragma) refuses the file: clang may
  // act on it (`#pragma clang attribute push (__attribute__((weak)), apply_to = ...)`, `#pragma GCC ...`) while EDG records
  // nothing, so the verified code would not be what runs (0011 item 7, 0017 item 2).  EDG's own diagnostic pragmas
  // (`#pragma diag_suppress 161`) could turn these errors off again; EDG keeps no IL entry for them, so
  // install_diag_pragma_recorder() records them as EDG processes them and the encoder rejects each one in a verified file
  // (diagnostic_pragma_in_verify).
  edg.push_back("--diag_error=161,1675");
  // Character literals EDG truncates or gives an implementation-defined value are refused (decision 0023 item 8; P40):
  // #1422 (a multi-character literal such as 'ab': an implementation-defined value, `-Wmultichar` in both compilers),
  // #1654 ('abcde': the leading characters are dropped), #26 (`L'ab'`, `u'ab'`, `U'ab'` keep only their first character in
  // C; already an error in C++) and #3255 (C: a UTF-16 literal such as `u'\U0001F600'` does not fit and is truncated; C++ has
  // a hard error #1535 of its own).  The diagnostic pragmas that could lower them again are rejected in verified files.
  edg.push_back("--diag_error=1422,1654,26,3255");
  // C: pointers of incompatible object types are warnings in EDG's C mode and errors in C++ (decision 0032 item 3; P56, audit round 4's
  // reopened item): #42 (`pi == pl`), #144 (`long *pl = pi;`, and `int *c = pc;` from a `const int *`), #513 (`pl = pi;`) and #167 (an
  // incompatible argument).  A conversion the front end accepts there would otherwise reach the encoder as a node it must reject.
  // C++ has #144 and #167 as discretionary warnings for exactly one shape, `int **` to `const int **` (an initializer, an argument), which clang++ and
  // g++ reject (it would let `*c = &constant` launder a const; decision 0040 item 2, P62 H1); every other #144/#167 there is already an error.
  if (c_mode) edg.push_back("--diag_error=42,144,167,513");
  else edg.push_back("--diag_error=144,167");
  for (const std::string& d : a.includes) edg.push_back("-I" + d);
  for (const std::string& m : a.macro_options) edg.push_back(m);
  edg.push_back(a.file);
  // EDG's diagnostics go to a file of ours first, so that the driver can tell an internal error from a missing
  // source file; they are copied to standard error as they are.
  std::string diag_dir;
  {
    std::error_code ec;
    std::filesystem::path base = std::filesystem::temp_directory_path(ec);
    if (ec) base = "/tmp";
    std::string templ = (base / "cerastes-diag-XXXXXX").string();
    std::vector<char> buf(templ.begin(), templ.end());
    buf.push_back('\0');
    if (::mkdtemp(buf.data()) == nullptr) {
      std::fprintf(stderr, "cerastes: error: cannot create a temporary directory\n");
      return 3;
    }
    diag_dir = buf.data();
  }
  const std::string diag_file = diag_dir + "/diagnostics.txt";
  std::snprintf(g_diag_dir, sizeof g_diag_dir, "%s", diag_dir.c_str());
  std::snprintf(g_diag_file, sizeof g_diag_file, "%s", diag_file.c_str());
  // (before the source file, which stays last)
  edg.insert(edg.end() - 1, {"--error_output", diag_file});
  std::vector<char*> edg_argv;
  for (std::string& s : edg) edg_argv.push_back(s.data());
  edg_argv.push_back(nullptr);

  cerastes::VerifyConfig& cfg = cerastes::verify_config();
  cfg.on = true;
  cfg.hook = a.hook;
  cfg.system_dirs = cerastes::system_directories(sys_includes, a.includes);
  cfg.cerastes_header = cerastes::canonical_path(std::string(CERASTES_TOOL_INCLUDE_DIR) + "/cerastes.h");

  // A system file is never the subject of a run (item 4): verifying one would check, and trust, nothing of ours.
  if (cerastes::is_system_file(cfg.system_dirs, cfg.cerastes_header, a.file)) {
    std::error_code ec;
    std::filesystem::remove_all(diag_dir, ec);
    return refuse("\"" + a.file + "\" is a system file: it is only ever used through its contracts, never verified");
  }
  // The kept Viper files must not overwrite the program (a mistyped --keep-vpr path, or a source named <stem>.vpr
  // when the default path is used).
  if (a.keep) {
    std::string keep = a.keep_path;
    if (keep.empty()) {
      std::string stem = a.file;
      size_t slash = stem.find_last_of('/');
      if (slash != std::string::npos) stem = stem.substr(slash + 1);
      size_t dot = stem.find_last_of('.');
      if (dot != std::string::npos && dot > 0) stem = stem.substr(0, dot);
      keep = stem + ".vpr";
    }
    const std::string src = cerastes::canonical_path(a.file);
    for (const std::string& k : {keep, keep + ".map"}) {
      if (k == a.file || (!src.empty() && cerastes::canonical_path(k) == src)) {
        std::error_code ec;
        std::filesystem::remove_all(diag_dir, ec);
        return refuse("--keep-vpr path " + keep + " would overwrite the source file");
      }
    }
  }

  scrub_environment();
  install_crash_handlers();
  install_terminate_handlers();
  cerastes::register_cerastes_annotations();
  cerastes::install_diag_pragma_recorder();
  const int edg_status = edg_main(static_cast<int>(edg_argv.size()) - 1, edg_argv.data());
  std::fflush(stderr);
  const std::string diagnostics = slurp(diag_file);
  {
    std::error_code ec;
    std::filesystem::remove_all(diag_dir, ec);
    g_diag_file[0] = g_diag_dir[0] = '\0';
  }
  std::fputs(diagnostics.c_str(), stderr);
  std::fflush(stderr);
  if (has_internal_error(diagnostics)) {
    std::fprintf(stderr, "cerastes: error: internal error in the front end (the program was not checked)\n");
    return 3;
  }
  if (cerastes::front_end_exit_status(cfg.out, edg_status) == 3) {
    // Decision 0022 item 12: lowering's internal errors, the emitter's problems and the encoder's "encoded twice" guard; their
    // diagnostics were printed above.
    std::fprintf(stderr, "cerastes: error: internal inconsistency in the encoder (the program was not checked)\n");
    return 3;
  }
  if (edg_status != 0) return 2;  // EDG's and cerastes's diagnostics are on standard error
  if (!cerastes::back_end_ran()) {
    std::fprintf(stderr, "cerastes: error: the front end finished without running the cerastes back end: nothing was checked\n");
    return 3;
  }
  if (!cfg.out.encoded) {
    std::fprintf(stderr, "cerastes: error: the encoder produced no program: nothing was checked\n");
    return 3;
  }

  if (cfg.out.functions == 0) {
    std::fprintf(stderr, "cerastes: error: %s: nothing to verify (no function with a body, and none called through a contract, in this file or its user headers)\n",
                 a.file.c_str());
    return 2;
  }

  cerastes::PipelineOptions po;
  po.tools = tools;
  po.hook = a.hook;
  po.timeout_seconds = a.hook == "silicon_timeout" ? 0 : a.timeout;
  po.primary_file = a.file;
  po.functions = cfg.out.functions;
  if (a.keep) {
    if (!a.keep_path.empty()) {
      po.keep_vpr = a.keep_path;
    } else {
      std::string stem = a.file;
      size_t slash = stem.find_last_of('/');
      if (slash != std::string::npos) stem = stem.substr(slash + 1);
      size_t dot = stem.find_last_of('.');
      if (dot != std::string::npos && dot > 0) stem = stem.substr(0, dot);
      po.keep_vpr = stem + ".vpr";
    }
  }
  cerastes::PipelineResult r = cerastes::run_pipeline(po, cfg.out.vpr, cfg.out.map);
  std::fputs(r.out.c_str(), stdout);
  std::fputs(r.err.c_str(), stderr);
  std::fflush(stdout);
  std::fflush(stderr);
  return r.exit_code;
}

// Prints what the Silicon-output classifier makes of a recorded output, for the harness self-test to compare with
// scripts/run_tests.py: one `error` line per entry, then one `verdict` line.
int classify_command(const char* path, const char* exit_code) {
  FILE* f = std::fopen(path, "rb");
  if (f == nullptr) return refuse(std::string("cannot read ") + path);
  std::string text;
  char buf[4096];
  size_t n;
  while ((n = std::fread(buf, 1, sizeof buf, f)) > 0) text.append(buf, n);
  std::fclose(f);
  cerastes::SiliconVerdict v = cerastes::parse_silicon_output(text, std::atoi(exit_code));
  for (const cerastes::SiliconError& e : v.errors)
    std::printf("error\t%s\t%s\t%s\t%s\t%d\n", e.viper_id.c_str(), e.category.c_str(), e.reason.c_str(), e.file.c_str(), e.line);
  std::printf("verdict\t%s\t%s\n", v.verified ? "verified" : (v.problem.empty() ? "errors" : "problem"), v.problem.c_str());
  return 0;
}

}  // namespace

int main(int argc, char** argv) {
  if (argc < 2) {
    std::fputs(kUsage, stderr);
    return 2;
  }
  const std::string cmd = argv[1];
  if (cmd == "--help" || cmd == "-h" || cmd == "help") {
    std::fputs(kUsage, stdout);
    return 0;
  }
  if (cmd == "--version") {
    std::printf("%s\n", kVersion);
    return 0;
  }
  if (cmd == "--selftest") {
    cerastes::SiliconTools tools{CERASTES_TOOL_JAVA, CERASTES_TOOL_SILICON_JAR, CERASTES_TOOL_Z3};
    return cerastes::run_selftest(tools);
  }
  if (cmd == "--cerastes_dump_configuration" && argc == 2) {
    // For scripts/check_edg_config.sh: EDG's own report of the configuration this binary was built with.
    ::setenv("EDG_BASE", CERASTES_TOOL_EDG_BASE, 1);
    static char a0[] = "cerastes", a1[] = "--dump_configuration";
    char* v[] = {a0, a1, nullptr};
    return edg_main(2, v);
  }
  if (cmd == "--cerastes_classify_silicon" && argc == 4) return classify_command(argv[2], argv[3]);
  if (cmd == "verify") {
    return verify_command(argc - 2, argv + 2);
  }
  return refuse("unknown command \"" + cmd + "\" (run `cerastes --help`)");
}

// Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#include "silicon.h"

#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cerrno>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <vector>

extern char** environ;

namespace cerastes {
namespace {

namespace fs = std::filesystem;

// What silicon_signal_cleanup() needs; plain data, set before the child exists and cleared after it is reaped.
volatile sig_atomic_t g_pgid = 0;
char g_file[1024] = "";
char g_dir[1024] = "";

struct TempDir {
  std::string path;
  ~TempDir() {
    if (path.empty()) return;
    std::error_code ec;
    fs::remove_all(path, ec);
  }
};

}  // namespace

SiliconRun run_silicon(const SiliconTools& tools, const std::string& vpr, int timeout_seconds) {
  SiliconRun run;
  run.vpr_name = "program.vpr";

  // A private directory (mode 0700): Silicon and Z3 may write there, and nothing else shares it.
  std::error_code ec;
  fs::path base = fs::temp_directory_path(ec);
  if (ec) base = "/tmp";
  std::string templ = (base / "cerastes-silicon-XXXXXX").string();
  std::vector<char> buf(templ.begin(), templ.end());
  buf.push_back('\0');
  if (::mkdtemp(buf.data()) == nullptr) {
    run.error = std::string("cannot create a temporary directory: ") + std::strerror(errno);
    return run;
  }
  TempDir dir;
  dir.path = buf.data();
  const std::string file = dir.path + "/" + run.vpr_name;
  std::snprintf(g_dir, sizeof g_dir, "%s", dir.path.c_str());
  std::snprintf(g_file, sizeof g_file, "%s", file.c_str());
  {
    std::ofstream out(file, std::ios::binary);
    out << vpr;
    out.flush();
    if (!out) {
      run.error = "cannot write " + file;
      return run;
    }
  }

  int fds[2];
  if (::pipe(fds) != 0) {
    run.error = std::string("pipe failed: ") + std::strerror(errno);
    return run;
  }
  ::fcntl(fds[0], F_SETFD, FD_CLOEXEC);
  ::fcntl(fds[1], F_SETFD, FD_CLOEXEC);

  posix_spawn_file_actions_t fa;
  posix_spawn_file_actions_init(&fa);
  posix_spawn_file_actions_addopen(&fa, 0, "/dev/null", O_RDONLY, 0);
  posix_spawn_file_actions_adddup2(&fa, fds[1], 1);
  posix_spawn_file_actions_adddup2(&fa, fds[1], 2);
#if defined(__APPLE__)
  // addchdir_np is deprecated from macOS 26 in favour of addchdir.
  if (__builtin_available(macOS 26.0, *)) {
    posix_spawn_file_actions_addchdir(&fa, dir.path.c_str());
  } else {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    posix_spawn_file_actions_addchdir_np(&fa, dir.path.c_str());
#pragma clang diagnostic pop
  }
#else
  // glibc has addchdir_np from 2.29 (decision 0031 item 4); its result is checked when the child starts, as on macOS.
  posix_spawn_file_actions_addchdir_np(&fa, dir.path.c_str());
#endif

  posix_spawnattr_t attr;
  posix_spawnattr_init(&attr);
  sigset_t none, all;
  sigemptyset(&none);
  sigfillset(&all);
  posix_spawnattr_setsigmask(&attr, &none);
  posix_spawnattr_setsigdefault(&attr, &all);
  posix_spawnattr_setpgroup(&attr, 0);  // its own process group, so that a timeout reaches Z3 too
  posix_spawnattr_setflags(&attr, POSIX_SPAWN_SETSIGMASK | POSIX_SPAWN_SETSIGDEF | POSIX_SPAWN_SETPGROUP);

  std::vector<std::string> argv_s = {tools.java, "-Xss128m", "-jar", tools.jar, "--z3Exe", tools.z3,
                                     "--numberOfErrorsToReport", "0", run.vpr_name};
  std::vector<char*> argv;
  for (std::string& s : argv_s) argv.push_back(s.data());
  argv.push_back(nullptr);
  // A clean environment: a stray JAVA_TOOL_OPTIONS or Z3 setting cannot change a result (0011 item 3).
  std::vector<std::string> env_s = {"PATH=/usr/bin:/bin", "HOME=" + dir.path, "TMPDIR=" + dir.path,
                                    "LC_ALL=en_US.UTF-8"};
  std::vector<char*> envp;
  for (std::string& s : env_s) envp.push_back(s.data());
  envp.push_back(nullptr);

  pid_t pid = 0;
  int rc = posix_spawn(&pid, tools.java.c_str(), &fa, &attr, argv.data(), envp.data());
  posix_spawn_file_actions_destroy(&fa);
  posix_spawnattr_destroy(&attr);
  ::close(fds[1]);
  if (rc != 0) {
    ::close(fds[0]);
    run.error = "cannot run " + tools.java + ": " + std::strerror(rc);
    return run;
  }
  run.started = true;
  g_pgid = pid;

  ::fcntl(fds[0], F_SETFL, ::fcntl(fds[0], F_GETFL) | O_NONBLOCK);
  const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(timeout_seconds);
  auto drain = [&] {
    char chunk[8192];
    for (;;) {
      ssize_t got = ::read(fds[0], chunk, sizeof chunk);
      if (got > 0) run.output.append(chunk, static_cast<size_t>(got));
      else break;  // end of file, EAGAIN, or an error: nothing more to read now
    }
  };
  int status = 0;
  bool reaped = false;
  // Poll until Silicon itself has exited or the deadline passes.  End of file on the pipe is not waited for: a Z3
  // that outlives Silicon may hold the write end open (it is killed with the group below).
  for (;;) {
    // Silicon has exited when waitid says so without reaping it (WNOWAIT): the group is then killed BEFORE the
    // child is reaped, while its process group id is still ours, and only then waited for.
    siginfo_t info;
    info.si_pid = 0;
    int w = ::waitid(P_PID, static_cast<id_t>(pid), &info, WEXITED | WNOHANG | WNOWAIT);
    if (w == 0 && info.si_pid == pid) {
      drain();
      ::killpg(pid, SIGKILL);
      while (::waitpid(pid, &status, 0) < 0 && errno == EINTR) {
      }
      reaped = true;
      break;
    }
    if (w < 0 && errno != EINTR) break;  // cannot wait for it: treated as a failure below
    if (std::chrono::steady_clock::now() >= deadline) {
      run.timed_out = true;
      break;
    }
    struct pollfd p = {fds[0], POLLIN, 0};
    ::poll(&p, 1, 50);
    drain();
  }
  // On a timeout (or a failed wait) the child is still ours: kill the group before waiting for it.
  if (!reaped) ::killpg(pid, SIGKILL);
  ::close(fds[0]);
  if (!reaped) {
    while (::waitpid(pid, &status, 0) < 0 && errno == EINTR) {
    }
    reaped = true;
  }
  g_pgid = 0;
  g_file[0] = g_dir[0] = '\0';
  if (!run.timed_out) {
    if (WIFEXITED(status)) run.exit_code = WEXITSTATUS(status);
    else if (WIFSIGNALED(status)) run.signaled = true;
  }
  return run;
}

void silicon_signal_cleanup() {
  const pid_t g = static_cast<pid_t>(g_pgid);
  if (g > 0) ::killpg(g, SIGKILL);
  if (g_file[0] != '\0') ::unlink(g_file);
  if (g_dir[0] != '\0') ::rmdir(g_dir);
}

}  // namespace cerastes

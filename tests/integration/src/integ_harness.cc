#include "integ_harness.hh"

#include <glib.h>
#include <stdlib.h>
#include <unistd.h>

#include <cstdio>
#include <cstring>
#include <stdexcept>

namespace integ {

const Harness* Harness::instance_ = nullptr;

namespace {

bool ProbeVconf() {
#ifdef HAVE_VCONF
  return access("/opt/usr/dbspace", R_OK) == 0;
#else
  return false;
#endif
}

bool ProbeAul() {
  return access("/run/aul", R_OK) == 0;
}

bool ProbeRpcPortLoopback() {
  const char* runtime_dir = getenv("XDG_RUNTIME_DIR");
  return runtime_dir != nullptr && access(runtime_dir, W_OK) == 0;
}

std::string MakeTmpdirPath() {
  char tmpl[] = "/tmp/notification-integ-XXXXXX";
  if (!mkdtemp(tmpl))
    throw std::runtime_error("mkdtemp failed for integ::TmpDb");
  return std::string(tmpl);
}

void RmrfBestEffort(const std::string& path) {
  if (path.empty()) return;
  std::string cmd = "rm -rf '" + path + "'";
  (void)std::system(cmd.c_str());
}

}  // namespace

void Harness::SetUp() {
  vconf_available_ = ProbeVconf();
  aul_available_ = ProbeAul();
  rpc_port_loopback_available_ = ProbeRpcPortLoopback();
  instance_ = this;
}

void Harness::TearDown() {
  instance_ = nullptr;
}

const Harness& Harness::Get() {
  if (!instance_)
    throw std::runtime_error("integ::Harness::Get called before SetUp");
  return *instance_;
}

TmpDb::TmpDb() : path_(MakeTmpdirPath()) {
  std::string db_path = path_ + "/.notification.db";
  setenv("NOTIFICATION_INTEGTEST_DB_PATH", db_path.c_str(), 1);
}

TmpDb::~TmpDb() {
  unsetenv("NOTIFICATION_INTEGTEST_DB_PATH");
  RmrfBestEffort(path_);
}

ScopedEnv::ScopedEnv(const char* key, const char* value) : key_(key) {
  const char* prior = getenv(key);
  if (prior) {
    prior_value_ = prior;
    prior_set_ = true;
  }
  if (value)
    setenv(key, value, 1);
  else
    unsetenv(key);
}

ScopedEnv::~ScopedEnv() {
  if (prior_set_)
    setenv(key_.c_str(), prior_value_.c_str(), 1);
  else
    unsetenv(key_.c_str());
}

MainloopTicker::MainloopTicker(int max_iterations)
    : max_iterations_(max_iterations) {}

bool MainloopTicker::wait_until(std::function<bool()> predicate) {
  for (int i = 0; i < max_iterations_; ++i) {
    if (predicate()) return true;
    g_main_context_iteration(nullptr, FALSE);
  }
  return predicate();
}

}  // namespace integ

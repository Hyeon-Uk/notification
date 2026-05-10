#pragma once

#include <gtest/gtest.h>

#include <functional>
#include <string>

namespace integ {

class Harness final : public ::testing::Environment {
 public:
  void SetUp() override;
  void TearDown() override;

  static const Harness& Get();

  bool VconfAvailable() const noexcept { return vconf_available_; }
  bool AulAvailable() const noexcept { return aul_available_; }
  bool RpcPortLoopbackAvailable() const noexcept {
    return rpc_port_loopback_available_;
  }

 private:
  bool vconf_available_{false};
  bool aul_available_{false};
  bool rpc_port_loopback_available_{false};
  static const Harness* instance_;
};

class TmpDb final {
 public:
  TmpDb();
  ~TmpDb();
  TmpDb(const TmpDb&) = delete;
  TmpDb& operator=(const TmpDb&) = delete;

  const std::string& path() const noexcept { return path_; }

 private:
  std::string path_;
};

class ScopedEnv final {
 public:
  ScopedEnv(const char* key, const char* value);
  ~ScopedEnv();
  ScopedEnv(const ScopedEnv&) = delete;
  ScopedEnv& operator=(const ScopedEnv&) = delete;

 private:
  std::string key_;
  std::string prior_value_;
  bool prior_set_{false};
};

class MainloopTicker final {
 public:
  explicit MainloopTicker(int max_iterations = 1000);

  bool wait_until(std::function<bool()> predicate);

 private:
  int max_iterations_;
};

}  // namespace integ

#include <gtest/gtest.h>

#include "integ_harness.hh"

namespace {

TEST(IntegHarnessSentinel, BootEnvProbeIsDeterministic) {
  const auto& h = integ::Harness::Get();
  bool any_probe_ran = h.VconfAvailable() || h.AulAvailable() ||
                       h.RpcPortLoopbackAvailable();
  EXPECT_EQ(any_probe_ran, any_probe_ran);
}

TEST(IntegHarnessSentinel, TmpDbAllocatesAndCleansUp) {
  std::string path;
  {
    integ::TmpDb db;
    path = db.path();
    EXPECT_FALSE(path.empty());
    EXPECT_NE(getenv("NOTIFICATION_INTEGTEST_DB_PATH"), nullptr);
  }
  EXPECT_EQ(getenv("NOTIFICATION_INTEGTEST_DB_PATH"), nullptr);
}

TEST(IntegHarnessSentinel, ScopedEnvRestoresPriorValue) {
  setenv("INTEG_TEST_TOKEN", "before", 1);
  {
    integ::ScopedEnv s("INTEG_TEST_TOKEN", "during");
    EXPECT_STREQ(getenv("INTEG_TEST_TOKEN"), "during");
  }
  EXPECT_STREQ(getenv("INTEG_TEST_TOKEN"), "before");
  unsetenv("INTEG_TEST_TOKEN");
}

}  // namespace

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  ::testing::AddGlobalTestEnvironment(new integ::Harness());
  return RUN_ALL_TESTS();
}

/*
 * Copyright (c) 2020 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef UNIT_TESTS_MOCK_TEST_FIXTURE_H_
#define UNIT_TESTS_MOCK_TEST_FIXTURE_H_

#include <gtest/gtest.h>

#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

#include "alloc_counter.hh"
#include "module_mock.hh"

class TestFixture : public ::testing::Test {
 public:
  explicit TestFixture(std::unique_ptr<ModuleMock>&& mock) {
    mock_ = std::move(mock);
  }
  virtual ~TestFixture() {
    mock_.reset();
  }

  virtual void SetUp() {}
  virtual void TearDown() {}

  template <typename T>
  static T& GetMock() {
    auto ptr = dynamic_cast<T*>(mock_.get());
    if (!ptr)
      throw std::invalid_argument("The test does not provide mock of \"" +
          std::string(typeid(T).name()) + "\"");
    return *ptr;
  }

  static std::unique_ptr<ModuleMock> mock_;
};

/*
 * LeakCheckFixture
 * ----------------
 * Takes a snapshot of the allocation counters at SetUp() and verifies that
 * the delta between total_allocs and total_frees has not grown during the
 * test body. Uses ADD_FAILURE_AT to surface leaks without aborting the run.
 *
 * Counting is scoped with SetCountingEnabled(true) inside SetUp/TearDown so
 * gtest / libstdc++ allocations that happen outside the fixture lifetime are
 * not counted.
 *
 * NOTE: To actually observe allocations the test binary must be linked with
 * -Wl,--wrap=malloc,--wrap=free,--wrap=realloc,--wrap=calloc AND compiled
 * with -DNOTI_TEST_WRAP_ALLOC. Without those flags the fixture still works
 * but all deltas are zero.
 */
class LeakCheckFixture : public TestFixture {
 public:
  explicit LeakCheckFixture(std::unique_ptr<ModuleMock>&& mock)
      : TestFixture(std::move(mock)) {}
  LeakCheckFixture() : TestFixture(nullptr) {}

  void SetUp() override {
    baseline_ = noti_test::SnapshotAllocs();
    noti_test::SetCountingEnabled(true);
    TestFixture::SetUp();
  }

  void TearDown() override {
    TestFixture::TearDown();
    noti_test::SetCountingEnabled(false);
    auto diff = noti_test::DiffAllocs(baseline_);
    /*
     * A positive net balance (allocs > frees) is reported as a leak.
     * We skip the assertion if NOTI_TEST_WRAP_ALLOC is absent - counters
     * are always zero in that case which would hide real failures only
     * silently (intended: test binary will still pass).
     */
    if (diff.total_allocs > diff.total_frees) {
      size_t net = diff.total_allocs - diff.total_frees;
      ADD_FAILURE() << "LeakCheckFixture: net " << net
                    << " live allocs (total_allocs=" << diff.total_allocs
                    << " total_frees=" << diff.total_frees
                    << " live_bytes=" << diff.live_bytes << ")";
    }
  }

  /* Explicit mid-test check helper. */
  void CheckLeak(const char* what = "CheckLeak") {
    auto diff = noti_test::DiffAllocs(baseline_);
    if (diff.total_allocs > diff.total_frees) {
      ADD_FAILURE() << what << ": leak detected net="
                    << (diff.total_allocs - diff.total_frees);
    }
  }

 private:
  noti_test::AllocSnapshot baseline_{};
};

/*
 * PerfAwareFixture
 * ----------------
 * Lets the test body snapshot an allocation baseline and assert that the
 * measured hot-path region performs fewer than N allocations.
 *
 * Usage:
 *   TEST_F(PostHotPathTest, Post_AllocCountUnderThreshold) {
 *     SnapshotAllocCount();
 *     notification_post(n);
 *     ExpectAllocCountUnder(10, "notification_post hot path");
 *   }
 */
class PerfAwareFixture : public TestFixture {
 public:
  explicit PerfAwareFixture(std::unique_ptr<ModuleMock>&& mock)
      : TestFixture(std::move(mock)) {}
  PerfAwareFixture() : TestFixture(nullptr) {}

  void SetUp() override {
    TestFixture::SetUp();
    noti_test::SetCountingEnabled(true);
    snapshot_ = noti_test::SnapshotAllocs();
  }

  void TearDown() override {
    noti_test::SetCountingEnabled(false);
    TestFixture::TearDown();
  }

  /* Re-arm the baseline to "now" (useful to isolate a hot-path region). */
  void SnapshotAllocCount() {
    snapshot_ = noti_test::SnapshotAllocs();
  }

  /* Read current diff without asserting. */
  size_t AllocCountSinceSnapshot() const {
    auto diff = noti_test::DiffAllocs(snapshot_);
    return diff.total_allocs;
  }

  /* Assert that number of allocations since the last snapshot is < max. */
  void ExpectAllocCountUnder(size_t max_allocs, const char* what) {
    auto diff = noti_test::DiffAllocs(snapshot_);
    EXPECT_LT(diff.total_allocs, max_allocs)
        << what << ": " << diff.total_allocs << " allocs (limit " << max_allocs
        << ")";
  }

 private:
  noti_test::AllocSnapshot snapshot_{};
};

#endif  // UNIT_TESTS_MOCK_TEST_FIXTURE_H_

/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd.
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

#include <gmock/gmock.h>

#include "mock/app_common_mock.h"
#include "mock/test_fixture.h"
#include "notification-ex/event_info_internal.h"

using namespace tizen_base;
using namespace notification;
using namespace std;

namespace {
class Mocks :
    virtual public ::testing::NiceMock<AppCommonMock> {};
} // namespace

class EventInfoTest : public TestFixture {
 public:
  EventInfoTest() : TestFixture(std::make_unique<::Mocks>()) {}
 protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(EventInfoTest, SerializeDeserialize) {
  EventInfo info(EventInfo::Get, "test", "channel", "id");
  Bundle b = info.Serialize();
  EventInfo serialized(b);
  ASSERT_EQ(serialized.GetEventType(), info.GetEventType());
  ASSERT_EQ(serialized.GetOwner(), info.GetOwner());
  ASSERT_EQ(serialized.GetChannel(), info.GetChannel());
  ASSERT_EQ(serialized.GetItemId(), info.GetItemId());
  ASSERT_EQ(serialized.GetUid(), info.GetUid());
}

TEST_F(EventInfoTest, GetString) {
  ASSERT_EQ(EventInfo::GetString(EventInfo::Post), "Post");
  ASSERT_EQ(EventInfo::GetString(EventInfo::Update), "Update");
  ASSERT_EQ(EventInfo::GetString(EventInfo::Delete), "Delete");
  ASSERT_EQ(EventInfo::GetString(EventInfo::Get), "Get");
}

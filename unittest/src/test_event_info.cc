// Copyright (c) 2019 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a apache 2.0 license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>

#include "notification-ex/event_info_internal.h"

using namespace tizen_base;
using namespace notification;
using namespace std;

namespace {

class EventInfoTest : public ::testing::Test {
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

}  // namespace

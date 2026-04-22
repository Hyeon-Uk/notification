/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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
 */

/*
 * N1-T2: FactoryManager shim migration unit tests.
 *
 * Covers the free-function entry points introduced in
 * default_item_factory.{h,cc} and default_action_factory.{h,cc}:
 *
 *   1. CreateItem_AllTypesSucceed         - every AbstractItem::Type
 *                                           except NullObject produces a
 *                                           non-null unique_ptr.
 *   2. CreateAction_AllTypesSucceed       - AppControl / Visibility
 *                                           produce non-null unique_ptrs;
 *                                           Custom returns nullptr per
 *                                           historical contract.
 *   3. GetNullItem_SingletonReturnsSame   - GetNullItem() returns the
 *                                           same address across calls and
 *                                           its address matches the one
 *                                           surfaced via the FactoryManager
 *                                           deprecation shim.
 *
 * These tests exercise the free-function path directly (no
 * FactoryManager singleton involvement), and one assertion also pins the
 * invariant that the FactoryManager shim delegates to the same
 * GetNullItem() singleton (ABI contract).
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <cstring>
#include <memory>

#include "mock/app_common_mock.hh"
#include "mock/aul_mock.hh"
#include "mock/glib_mock.hh"
#include "mock/test_fixture.hh"

#include "notification-ex/abstract_action.h"
#include "notification-ex/abstract_item.h"
#include "notification-ex/default_action_factory.h"
#include "notification-ex/default_item_factory.h"
#include "notification-ex/factory_manager.h"

using namespace notification;
using namespace notification::item;

using ::testing::_;
using ::testing::Invoke;

namespace {
class Mocks :
    virtual public ::testing::NiceMock<AulMock>,
    virtual public ::testing::NiceMock<AppCommonMock>,
    virtual public ::testing::NiceMock<GlibMock> {};

/* GroupItem construction calls app_get_name(&name) and, on success,
 * free(name). Without a proper stub the default NiceMock returns 0 and
 * leaves name uninitialized, causing munmap_chunk() crashes. */
int FakeAppGetName(char** app_name) {
  *app_name = strdup("unittest_appname");
  return 0;
}
}  // namespace

class FactoryMigrationTest : public TestFixture {
 public:
  FactoryMigrationTest() : TestFixture(std::make_unique<::Mocks>()) {}

  void SetUp() override {
    EXPECT_CALL(GetMock<AppCommonMock>(), app_get_name(_))
        .WillRepeatedly(Invoke(FakeAppGetName));
  }
};

TEST_F(FactoryMigrationTest, CreateItem_AllTypesSucceed) {
  /* 12 non-null types defined by AbstractItem::Type. Custom is mapped to
   * ButtonItem by the default factory. NullObject is intentionally not
   * constructible via CreateItem (throws ERROR_INVALID_PARAMETER) and is
   * covered by the GetNullItem case below. */
  const int types[] = {
    AbstractItem::Text,
    AbstractItem::Image,
    AbstractItem::Icon,
    AbstractItem::Button,
    AbstractItem::ChatMessage,
    AbstractItem::CheckBox,
    AbstractItem::InputSelector,
    AbstractItem::Group,
    AbstractItem::Entry,
    AbstractItem::Progress,
    AbstractItem::Time,
    AbstractItem::Custom,
  };

  for (int t : types) {
    auto item = ::notification::item::CreateItem(t);
    ASSERT_NE(item.get(), nullptr) << "type=" << t;
  }
}

TEST_F(FactoryMigrationTest, CreateAction_AllTypesSucceed) {
  auto app_ctl = ::notification::item::CreateAction(AbstractAction::AppControl);
  ASSERT_NE(app_ctl.get(), nullptr);
  ASSERT_EQ(app_ctl->GetType(), AbstractAction::AppControl);

  auto vis = ::notification::item::CreateAction(AbstractAction::Visibility);
  ASSERT_NE(vis.get(), nullptr);
  ASSERT_EQ(vis->GetType(), AbstractAction::Visibility);

  /* Custom returns nullptr in the default dispatch (historical
   * contract inherited from DefaultActionFactory). Keep the invariant. */
  auto custom = ::notification::item::CreateAction(AbstractAction::Custom);
  ASSERT_EQ(custom.get(), nullptr);
}

TEST_F(FactoryMigrationTest, GetNullItem_SingletonReturnsSameAddress) {
  AbstractItem& a = ::notification::item::GetNullItem();
  AbstractItem& b = ::notification::item::GetNullItem();
  ASSERT_EQ(&a, &b);

  /* ABI pin: FactoryManager shim must delegate to the same singleton so
   * callers that still use the shim observe identical identity. */
  AbstractItem& via_shim = FactoryManager::GetInst().GetNullItem();
  ASSERT_EQ(&a, &via_shim);

  /* Type sanity: the shared null must report NullObject. */
  ASSERT_EQ(a.GetType(), AbstractItem::NullObject);
}

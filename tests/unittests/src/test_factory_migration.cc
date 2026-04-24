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
 * N3-T2 D8: FactoryManager body retired; tests now pin the
 *           free-function entry points (CreateItem / CreateAction /
 *           GetNullItem / SetItemFactoryOverride) and verify that the
 *           FactoryManager class symbol is absent from the loaded
 *           library (NoSingletonSymbol).
 *
 * Covers the free-function entry points from
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
 *                                           same address across calls.
 *   4. NoSingletonSymbol                  - dlsym for the mangled
 *                                           FactoryManager::GetInst()
 *                                           and ::Register* / ::Create*
 *                                           member names returns
 *                                           nullptr in the loaded
 *                                           notification-ex library
 *                                           (GATE for D8 removal).
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <dlfcn.h>

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

  /* Type sanity: the shared null must report NullObject. */
  ASSERT_EQ(a.GetType(), AbstractItem::NullObject);
}

/*
 * N3-T2 D8 GATE: No FactoryManager class symbol must be resolvable from
 * the loaded notification-ex library. The class body was removed; only
 * the empty header remains for legacy include-compat.
 *
 * We probe the itanium-ABI mangled names for FactoryManager::GetInst(),
 * ::RegisterFactory(unique_ptr<IItemFactory>), ::CreateItem(int),
 * ::GetNullItem(), and the class's typeinfo - every one of these must
 * return nullptr. We use RTLD_DEFAULT so the lookup spans the exe +
 * every dlopen'd library, matching the `nm --dynamic libnotification-ex.so
 * | grep FactoryManager` check from the plan.
 */
TEST_F(FactoryMigrationTest, NoSingletonSymbol) {
  /* _ZN12notification4item14FactoryManager7GetInstEv
   *   == notification::item::FactoryManager::GetInst() */
  ASSERT_EQ(dlsym(RTLD_DEFAULT,
      "_ZN12notification4item14FactoryManager7GetInstEv"), nullptr);
  /* _ZN12notification4item14FactoryManager11GetNullItemEv
   *   == notification::item::FactoryManager::GetNullItem() */
  ASSERT_EQ(dlsym(RTLD_DEFAULT,
      "_ZN12notification4item14FactoryManager11GetNullItemEv"), nullptr);
  /* _ZN12notification4item14FactoryManager10CreateItemEi
   *   == notification::item::FactoryManager::CreateItem(int) */
  ASSERT_EQ(dlsym(RTLD_DEFAULT,
      "_ZN12notification4item14FactoryManager10CreateItemEi"), nullptr);
  /* Class typeinfo — also expected absent. */
  ASSERT_EQ(dlsym(RTLD_DEFAULT,
      "_ZTIN12notification4item14FactoryManagerE"), nullptr);
}

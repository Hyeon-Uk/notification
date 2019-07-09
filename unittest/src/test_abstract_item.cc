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

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <app_control.h>

#include "notification-ex/item_inflator.h"
#include "notification-ex/app_control_action.h"
#include "notification-ex/item_info_internal.h"
#include "notification-ex/iitem_factory.h"
#include "notification-ex/factory_manager.h"
#include "notification-ex/default_item_factory.h"

#define MY_ITEM_TYPE AbstractItem::Type::Custom + 1

using namespace notification;
using namespace tizen_base;
using namespace notification::item;

namespace {
class TestItem : public AbstractItem {
  public:
   TestItem(std::shared_ptr<AbstractAction> action = std::shared_ptr<AbstractAction>({}))
     : AbstractItem(action) {
   }
   TestItem(std::string id, std::shared_ptr<AbstractAction> action = std::shared_ptr<AbstractAction>({}))
     : AbstractItem(id, action) {
   }
   virtual ~TestItem() {}

  Bundle Serialize() const override {
    Bundle b;
    b = AbstractItem::Serialize();
    return b;
  }
  void Deserialize(Bundle b) override {
    AbstractItem::Deserialize(b);
  }
  AbstractItem& FindByID(std::string id) override {
    return *this;
  }
  bool IsItemTypeExist(int type) {
  if (GetType() == type)
      return true;
    return false;
  }
  int GetType() const override {
    return MY_ITEM_TYPE;
  }
};

class MyFactory : public IItemFactory {
 public:
  MyFactory() {}
  virtual ~MyFactory() {}

  std::unique_ptr<AbstractItem> CreateItem(int type) override {
    if (type == MY_ITEM_TYPE)
      return std::unique_ptr<AbstractItem>(new TestItem(""));

    return nullptr;
  }
};

class AbstractItemTest : public ::testing::Test {
 public:
  virtual void SetUp() {
    FactoryManager::GetInst().RegisterFactory(std::unique_ptr<IItemFactory>(new MyFactory()));
  }
  virtual void TearDown() {
    FactoryManager::GetInst().RegisterFactory(std::unique_ptr<IItemFactory>(new DefaultItemFactory()));
  }
};

TEST_F(AbstractItemTest, SerializeDeserialize) {
  /* Serialize */
  app_control_h app_control, app_control_1;
  char* app_id = NULL;
  time_t current_time;

  app_control_create(&app_control);
  app_control_set_app_id(app_control, "new_appid");
  std::shared_ptr<AppControlAction> action = std::make_shared<AppControlAction>(app_control);
  app_control_destroy(app_control);

  app_control_create(&app_control_1);
  app_control_set_app_id(app_control_1, "new_appid_1");
  std::shared_ptr<AppControlAction> action_1 = std::make_shared<AppControlAction>(app_control_1);
  app_control_destroy(app_control_1);

  TestItem item("test_id", action);

  std::shared_ptr<Color> color = std::make_shared<Color>(50, 100,150,200);
  std::shared_ptr<Padding> padding = std::make_shared<Padding>(10, 20, 30, 40);
  std::shared_ptr<Geometry> geometry = std::make_shared<Geometry>(110, 120, 130, 140);
  std::shared_ptr<Color> bg_color = std::make_shared<Color>(71, 72, 73, 74);

  item.SetStyle(std::make_shared<Style>(color, padding, geometry, bg_color, "bg path"));
  item.SetVisible(false);
  item.SetEnable(false);
  item.AddReceiver("receiver_1");
  item.AddReceiver("receiver_2");
  std::static_pointer_cast<IItemInfoInternal>(item.GetInfo())->AddHideViewer("hide_1");
  std::static_pointer_cast<IItemInfoInternal>(item.GetInfo())->AddHideViewer("hide_2");
  item.SetPolicy(AbstractItem::Policy::OnBootClear);
  std::static_pointer_cast<IItemInfoInternal>(item.GetInfo())->SetVersion(3);
  item.GetInfo()->SetHideTime(5);
  item.GetInfo()->SetDeleteTime(9);
  item.SetChannel("channel99");
  item.SetSoundPath("soundpath");
  item.SetVibrationPath("vibrationpath");
  std::static_pointer_cast<IItemInfoInternal>(item.GetInfo())->SetUid(3);
  item.SetSenderAppId("sender");
  item.SetTag("tag");

  time(&current_time);
  std::static_pointer_cast<IItemInfoInternal>(item.GetInfo())->SetTime(current_time);

  std::shared_ptr<Color> color2 = std::make_shared<Color>(150, 160, 170, 180);
  std::shared_ptr<LEDInfo> led = std::make_shared<LEDInfo>(color2);
  led->SetOnPeriod(10);
  led->SetOffPeriod(20);

  item.SetLEDInfo(led);
  item.SetOnGoingState(true);

  /* Deserialize */
  Bundle b = item.Serialize();

  std::shared_ptr<AbstractItem> gen_item = ItemInflator::Create(b);
  TestItem* gen_test = static_cast<TestItem*>(gen_item.get());

  ASSERT_EQ(gen_test->GetId(), "test_id");
  ASSERT_EQ(gen_test->GetType(), MY_ITEM_TYPE);
  ASSERT_EQ(std::static_pointer_cast<IItemInfoInternal>(gen_test->GetInfo())->GetUid(), 3);
  ASSERT_EQ(gen_test->GetEnable(), false);
  ASSERT_EQ(gen_test->GetVisible(),false);
  ASSERT_EQ(gen_test->GetPolicy(), AbstractItem::Policy::OnBootClear);
  ASSERT_EQ(std::static_pointer_cast<IItemInfoInternal>(gen_test->GetInfo())->GetVersion(), 3);
  ASSERT_EQ(gen_test->GetInfo()->GetHideTime(), 5);
  ASSERT_EQ(gen_test->GetInfo()->GetDeleteTime(), 9);
  ASSERT_EQ(gen_test->GetChannel(), "channel99");
  ASSERT_EQ(gen_test->GetSoundPath(), "soundpath");
  ASSERT_EQ(gen_test->GetVibrationPath(), "vibrationpath");
  ASSERT_EQ(gen_test->GetSenderAppId(), "sender");
  ASSERT_EQ(gen_test->GetTag(), "tag");
  ASSERT_EQ(gen_test->GetInfo()->GetTime(), current_time);

  ASSERT_EQ(gen_test->GetStyle()->GetPadding()->GetLeft(), 10);
  ASSERT_EQ(gen_test->GetStyle()->GetPadding()->GetTop(), 20);
  ASSERT_EQ(gen_test->GetStyle()->GetPadding()->GetRight(), 30);
  ASSERT_EQ(gen_test->GetStyle()->GetPadding()->GetBottom(), 40);

  ASSERT_EQ(gen_test->GetStyle()->GetColor()->GetAVal(), 50);
  ASSERT_EQ(gen_test->GetStyle()->GetColor()->GetRVal(), 100);
  ASSERT_EQ(gen_test->GetStyle()->GetColor()->GetGVal(), 150);
  ASSERT_EQ(gen_test->GetStyle()->GetColor()->GetBVal(), 200);

  ASSERT_EQ(gen_test->GetStyle()->GetGeometry()->GetX(), 110);
  ASSERT_EQ(gen_test->GetStyle()->GetGeometry()->GetY(), 120);
  ASSERT_EQ(gen_test->GetStyle()->GetGeometry()->GetWidth(), 130);
  ASSERT_EQ(gen_test->GetStyle()->GetGeometry()->GetHeight(), 140);

  ASSERT_EQ(gen_test->GetStyle()->GetBackgroundColor()->GetAVal(), 71);
  ASSERT_EQ(gen_test->GetStyle()->GetBackgroundColor()->GetRVal(), 72);
  ASSERT_EQ(gen_test->GetStyle()->GetBackgroundColor()->GetGVal(), 73);
  ASSERT_EQ(gen_test->GetStyle()->GetBackgroundColor()->GetBVal(), 74);

  ASSERT_EQ(gen_test->GetStyle()->GetBackgroundImage(), "bg path");

  ASSERT_EQ(gen_test->GetLEDInfo()->GetColor()->GetAVal(), 150);
  ASSERT_EQ(gen_test->GetLEDInfo()->GetColor()->GetRVal(), 160);
  ASSERT_EQ(gen_test->GetLEDInfo()->GetColor()->GetGVal(), 170);
  ASSERT_EQ(gen_test->GetLEDInfo()->GetColor()->GetBVal(), 180);
  ASSERT_EQ(gen_test->GetLEDInfo()->GetOnPeriod(), 10);
  ASSERT_EQ(gen_test->GetLEDInfo()->GetOffPeriod(), 20);

  std::list<std::string> receiver1 = item.GetReceiverList();
  std::list<std::string> receiver2 = gen_test->GetReceiverList();

  ASSERT_EQ(receiver1.size(), receiver2.size());

  for (unsigned int i = 0; i < receiver1.size(); i++) {
    ASSERT_EQ(receiver1.front(), receiver2.front());
    receiver1.pop_front();
    receiver2.pop_front();
  }

  std::list<std::string> hide1 =
      std::static_pointer_cast<IItemInfoInternal>(item.GetInfo())->GetHideViewerList();
  std::list<std::string> hide2 =
      std::static_pointer_cast<IItemInfoInternal>(gen_test->GetInfo())->GetHideViewerList();

  ASSERT_EQ(hide1.size(), hide2.size());

  for (unsigned int i = 0; i < hide1.size(); i++) {
    ASSERT_EQ(hide1.front(), hide2.front());
    hide1.pop_front();
    hide2.pop_front();
  }

  ASSERT_EQ(gen_test->GetAction()->GetType(), AbstractAction::Type::AppControl);

  std::shared_ptr<AppControlAction> ac =
      std::static_pointer_cast<AppControlAction>(gen_test->GetAction());
  app_control_get_app_id(ac->GetAppControl(), &app_id);

  ASSERT_STREQ(app_id, "new_appid");

  item.SetAction(action_1);

  b = item.Serialize();
  gen_item = ItemInflator::Create(b);
  gen_test = static_cast<TestItem*>(gen_item.get());

  ac = std::static_pointer_cast<AppControlAction>(gen_test->GetAction());
  app_control_get_app_id(ac->GetAppControl(), &app_id);

  ASSERT_STREQ(app_id, "new_appid_1");
  ASSERT_EQ(gen_test->GetOnGoingState(), true);
}

TEST_F(AbstractItemTest, SerializeDeserialize2) {
  /* Serialize */
  TestItem item("test_id");

  std::shared_ptr<Color> color = std::make_shared<Color>(50, 100,150,200);
  std::shared_ptr<Padding> padding = std::make_shared<Padding>(10, 20, 30, 40);
  std::shared_ptr<Geometry> geometry = std::make_shared<Geometry>(110, 120, 130, 140);

  item.SetStyle(std::make_shared<Style>(color, padding, geometry, nullptr, ""));

  /* Deserialize */
  Bundle b = item.Serialize();

  std::shared_ptr<AbstractItem> gen_item = ItemInflator::Create(b);
  TestItem* gen_test = static_cast<TestItem*>(gen_item.get());

  ASSERT_EQ(gen_test->GetStyle()->GetPadding()->GetLeft(), 10);
  ASSERT_EQ(gen_test->GetStyle()->GetPadding()->GetTop(), 20);
  ASSERT_EQ(gen_test->GetStyle()->GetPadding()->GetRight(), 30);
  ASSERT_EQ(gen_test->GetStyle()->GetPadding()->GetBottom(), 40);

  ASSERT_EQ(gen_test->GetStyle()->GetColor()->GetAVal(), 50);
  ASSERT_EQ(gen_test->GetStyle()->GetColor()->GetRVal(), 100);
  ASSERT_EQ(gen_test->GetStyle()->GetColor()->GetGVal(), 150);
  ASSERT_EQ(gen_test->GetStyle()->GetColor()->GetBVal(), 200);

  ASSERT_EQ(gen_test->GetStyle()->GetGeometry()->GetX(), 110);
  ASSERT_EQ(gen_test->GetStyle()->GetGeometry()->GetY(), 120);
  ASSERT_EQ(gen_test->GetStyle()->GetGeometry()->GetWidth(), 130);
  ASSERT_EQ(gen_test->GetStyle()->GetGeometry()->GetHeight(), 140);

  ASSERT_EQ(gen_test->GetStyle()->GetBackgroundColor(), nullptr);
  ASSERT_EQ(gen_test->GetStyle()->GetBackgroundImage(), "");
}

TEST_F(AbstractItemTest, ItemInfoCanReceive) {
  TestItem item("test_id");

  item.AddReceiver(ReceiverGroup::Panel);
  item.AddReceiver(ReceiverGroup::LockScreen);

  ASSERT_TRUE(std::static_pointer_cast<IItemInfoInternal>(item.GetInfo())->CanReceive(ReceiverGroup::Panel));
  ASSERT_TRUE(std::static_pointer_cast<IItemInfoInternal>(item.GetInfo())->CanReceive(ReceiverGroup::LockScreen));
  ASSERT_FALSE(std::static_pointer_cast<IItemInfoInternal>(item.GetInfo())->CanReceive(ReceiverGroup::Popup));
}

TEST_F(AbstractItemTest, SetGetOnGoingState) {
  TestItem item("test_id");

  ASSERT_EQ(item.GetOnGoingState(), false);

  item.SetOnGoingState(true);

  ASSERT_EQ(item.GetOnGoingState(), true);
}

}  // namespace
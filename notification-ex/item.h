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

#ifndef NOTIFICATION_EX_ITEM_H_
#define NOTIFICATION_EX_ITEM_H_

#include <time.h>

#include <memory>
#include <string>
#include <list>

#include "notification-ex/abstract_item.h"
#include "notification-ex/ex_bundle.h"

#ifndef EXPORT_API
#define EXPORT_API __attribute__((visibility("default")))
#endif

namespace notification {
namespace item {

class EXPORT_API TextItem : public AbstractItem {
 public:
  TextItem(std::string text);
  virtual ~TextItem();

  Bundle Serialize() override;
  void Deserialize(Bundle b) override;
  AbstractItem& FindByID(std::string id) override;
  void SetContents(std::string contents);
  std::string GetContents() const;
  std::string GetHyperLink() const;

 private:
  std::string text_ = nullptr;
};  // class TextItem

class EXPORT_API IconItem : public AbstractItem {
 public:
  IconItem(std::string iconpath);
  virtual ~IconItem();

  Bundle Serialize() override;
  void Deserialize(Bundle b) override;
  AbstractItem& FindByID(std::string id) override;

 private:
  std::string iconPath_ = nullptr;
};  // class IconItem

class EXPORT_API IconTextItem : public AbstractItem {
 public:
  IconTextItem(std::string iconpath, std::string text);
  virtual ~IconTextItem();

  Bundle Serialize() override;
  void Deserialize(Bundle b) override;
  AbstractItem& FindByID(std::string id) override;
  IconItem GetIconItem() const;
  TextItem GetTextItem() const;

 private:
  std::string iconPath_ = nullptr;
  std::string text_ = nullptr;
};  // class IconTextItem

class EXPORT_API ImageItem : public AbstractItem {
 public:
  ImageItem(std::string imagePath);
  virtual ~ImageItem();

  Bundle Serialize() override;
  void Deserialize(Bundle b) override;
  AbstractItem& FindByID(std::string id) override;
  std::string GetImagePath() const;

 private:
  std::string imagePath_ = nullptr;
};  // class ImageItem

class EXPORT_API ProgressItem : public AbstractItem {
 public:
  ProgressItem(float min, float max, float current);
  virtual ~ProgressItem();

  Bundle Serialize() override;
  void Deserialize(Bundle b) override;
  AbstractItem& FindByID(std::string id) override;
  float GetCurrent() const;
  void SetCurrent(float current);
  float GetMin() const;
  float GetMax() const;

 private:
  float min_ = 0.0;
  float max_ = 0.0;
  float current_ = 0.0;
};  // class ProgressItem

class EXPORT_API CheckBoxItem : public AbstractItem {
 public:
  CheckBoxItem(bool checked);
  virtual ~CheckBoxItem();

  Bundle Serialize() override;
  void Deserialize(Bundle b) override;
  AbstractItem& FindByID(std::string id) override;
  bool IsChecked() const;

private:
  bool checked_ = false;
};  // class CheckBoxItem

class EXPORT_API ChatMessageItem : public AbstractItem {
 public:
  enum Type {
    user,
    sender,
  };

 public:
  ChatMessageItem();
  virtual ~ChatMessageItem();

  Bundle Serialize() override;
  void Deserialize(Bundle b) override;
  AbstractItem& FindByID(std::string id) override;
  TextItem GetTextItem() const;
  TextItem GetDataItem() const;
  time_t GetTimeItem() const;
  Type GetType() const;
};  // class ChatMessageItem

class EXPORT_API EntryItem : public AbstractItem {
 public:
  EntryItem();
  virtual ~EntryItem();

  Bundle Serialize() override;
  void Deserialize(Bundle b) override;
  AbstractItem& FindByID(std::string id) override;
  std::string GetText() const;
  void SetText(std::string text);
  void SetTextLimit(int size);
  int GetTextLimit() const;

 private:
  std::string text_ = nullptr;
  int limit_ = 160;
};  // class EntryItem

class EXPORT_API EffectItem : public AbstractItem {
 public:
  EffectItem();
  virtual ~EffectItem();

  Bundle Serialize() override;
  void Deserialize(Bundle b) override;
  AbstractItem& FindByID(std::string id) override;
  std::string GetSoundPath() const;
  std::string GetVibrationPath() const;

 private:
  std::string soundPath_ = nullptr;
  std::string vibrationPath_ = nullptr;
};  // class EffectItem

class EXPORT_API CustomItem : public AbstractItem {
 public:
  CustomItem();
  virtual ~CustomItem();

  Bundle Serialize() override;
  void Deserialize(Bundle b) override;
  AbstractItem& FindByID(std::string id) override;
};  // class CustomItem

}  // namespace item
}  // nampace notification
#endif  // NOTIFICATION_EX_ITEM_H_

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

#ifndef NOTIFICATION_EX_ABSTRACT_ITEM_H_
#define NOTIFICATION_EX_ABSTRACT_ITEM_H_

#include <time.h>

#include <memory>
#include <string>

#include "notification-ex/abstract_action.h"
#include "notification-ex/ex_bundle.h"

#ifndef EXPORT_API
#define EXPORT_API __attribute__((visibility("default")))
#endif

namespace notification {
namespace item {

class EXPORT_API Color {
 public:
  Color() {
    a_ = 0;
    r_ = 0;
    g_ = 0;
    b_ = 0;
  }
  Color(unsigned char a, unsigned char r, unsigned char g, unsigned char b)
    : a_(a), r_(r), g_(g), b_(b) {
  }
  virtual ~Color() = default;

  unsigned char GetAVal() const {
    return a_;
  }
  unsigned char GetRVal() const {
    return r_;
  }
  unsigned char GetGVal() const {
    return g_;
  }
  unsigned char GetBVal() const {
    return b_;
  }

 private:
  unsigned char a_;
  unsigned char r_;
  unsigned char g_;
  unsigned char b_;
};  // class Color

class EXPORT_API Padding {
 public:
  Padding() {
    left_ = 0;
    top_ = 0;
    right_ = 0;
    bottom_ = 0;
  }
  Padding(int left, int top, int right, int bottom)
    : left_(left), top_(top), right_(right), bottom_(bottom) {
  }
  virtual ~Padding() = default;

  int GeLeft() const {
    return left_;
  }
  int GetTop() const {
    return top_;
  }
  int GetRight() const {
    return right_;
  }
  int GetBottom() const {
    return bottom_;
  }

 private:
  int left_;
  int top_;
  int right_;
  int bottom_;
};  // class  Padding

class EXPORT_API Geometry {
 public:
  Geometry() {
    x_ = 0;
    y_ = 0;
    w_ = 0;
    h_ = 0;
  }
  Geometry(int x, int y, int w, int h) : x_(x), y_(y), w_(w), h_(h) {
  }
  virtual ~Geometry() = default;

  int GetX() const {
    return x_;
  }
  int GetY() const {
    return y_;
  }
  int GetWidth() const {
    return w_;
  }
  int GetHeight() const {
    return h_;
  }

 private:
  int x_;
  int y_;
  int w_;
  int h_;
};  // class Geometry

class EXPORT_API Style {
 public:
  Style() {
  }
  Style(Color color, Padding padding, Geometry geometry);
  virtual ~Style() = default;

  Padding GetPadding() const;
  Color GetColor() const;
  Geometry GetGeometry() const;

 private:
  Color color_;
  Padding padding_;
  Geometry geometry_;
};  // class Style

class EXPORT_API LEDInfo {
 public:
  LEDInfo() {
    Color(0, 0, 0, 0);
  }
  explicit LEDInfo(Color color);
  virtual ~LEDInfo() = default;

  void SetOffPeriod(int ms);
  void SetOnPeriod(int ms);
  Color GetColor() const;

 private:
  Color color_;
  int period_ = 0;
};  // clss LEDInfo

class EXPORT_API AbstractItem {
 public:
  enum Type {
    Text,
    Image,
    Icon,
    Button,
    ChatMessage,
    CheckBox,
    IconText,
    InputSelector,
    Group,
    Effect,
    Progress,
    Custom,
  };

  enum Policy {
    None,
    OnBootClear,
    SimMode,
  };

 public:
  AbstractItem();
  AbstractItem(Type type,
      std::shared_ptr<AbstractAction> action = std::shared_ptr<AbstractAction>({}));
  AbstractItem(std::string id, Type type,
      std::shared_ptr<AbstractAction> action = std::shared_ptr<AbstractAction>({}));
  virtual ~AbstractItem() = 0;

  virtual Bundle Serialize() = 0;
  virtual void Deserialize(Bundle b) = 0;
  virtual std::shared_ptr<AbstractItem> FindByID(std::string id) = 0;
  std::string GetId() const;
  void SetId(std::string id);
  std::shared_ptr<AbstractAction> GetAction() const;
  void SetAction(std::shared_ptr<AbstractAction> action) const;
  std::shared_ptr<Style> GetStyle() const;
  void SetStyle(std::shared_ptr<Style> style);
  Type GetType() const;
  static AbstractItem::Type GetType(Bundle b);
  void SetVisible(bool visible);
  void SetEnable(bool enable);
  void AddReceiver(std::string receiver_group);
  void RemoveReceiver(std::string receiver_group);
  bool CanReceive(std::string id) const;
  void SetPolicy(Policy policy);
  Policy GetPolicy() const;
  int GetVersion() const;
  void SetVersion(int ver);
  int GetHideTime() const;
  int GetDeleteTime() const;
  std::string GetChannel() const;
  void SetChannel(std::string channel);
  void SetLEDInfo(std::shared_ptr<LEDInfo> led);
  std::shared_ptr<LEDInfo> GetLEDInfo() const;

 private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};  // class AbstractItem

}  // namespace item
}  // namespace notification

#endif  // NOTIFICATION_EX_ABSTRACT_ITEM_H_

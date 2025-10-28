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
#include <bundle_cpp.h>

#include <memory>
#include <string>
#include <list>
#include <vector>
#include <map>

#include "notification-ex/abstract_action.h"
#include "notification-ex/multi_language.h"
#include "notification-ex/iitem_info.h"

#ifndef EXPORT_API
#define EXPORT_API __attribute__((visibility("default")))
#endif

namespace notification {
namespace item {

/* LCOV_EXCL_START */
/**
 * @brief The class for ReceiverGroup.
 * @details The class to define receiver group of notification.
 * @since_tizen 5.5
 */
class EXPORT_API ReceiverGroup {
 public:
  static const std::string Panel;
  static const std::string Ticker;
  static const std::string LockScreen;
  static const std::string Indicator;
  static const std::string Popup;
};

/**
 * @brief The class for color data.
 * @details The color consists A,R,G,B values.
 * @since_tizen 5.5
 */
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

  /**
   * @brief Copy-constructor.
   * @since_tizen 5.5
   * @param[in] c The object to copy
   */
  Color(const Color& c) {
    a_ = c.a_;
    r_ = c.r_;
    g_ = c.g_;
    b_ = c.b_;
  }

  /**
   * @brief Assignment.
   * @since_tizen 5.5
   * @param[in] c The object to copy
   */
  Color& operator = (const Color& c) {
    if (this != &c) {
      a_ = c.a_;
      r_ = c.r_;
      g_ = c.g_;
      b_ = c.b_;
    }
    return *this;
  }

  /**
   * @brief Move-constructor.
   * @since_tizen 5.5
   * @param[in] c The object to move
   */
  Color(Color&& c) noexcept {
    a_ = c.a_;
    c.a_ = 0;

    r_ = c.r_;
    c.r_ = 0;

    g_ = c.g_;
    c.g_ = 0;

    b_ = c.b_;
    c.b_ = 0;
  }

  /**
   * @brief Assignment.
   * @since_tizen 5.5
   * @param[in] c The object to move
   */
  Color& operator = (Color&& c) noexcept {
    if (this != &c) {
      a_ = c.a_;
      c.a_ = 0;

      r_ = c.r_;
      c.r_ = 0;

      g_ = c.g_;
      c.g_ = 0;

      b_ = c.b_;
      c.b_ = 0;
    }
    return *this;
  }

  /**
   * @brief Gets alpha value of color.
   * @since_tizen 5.5
   * @return The alpha value of color
   */
  unsigned char GetAVal() const {
    return a_;
  }

  /**
   * @brief Gets red value of color.
   * @since_tizen 5.5
   * @return The red value of color
   */
  unsigned char GetRVal() const {
    return r_;
  }

  /**
   * @brief Gets green value of color.
   * @since_tizen 5.5
   * @return The green value of color
   */
  unsigned char GetGVal() const {
    return g_;
  }

  /**
   * @brief Gets blue value of color.
   * @since_tizen 5.5
   * @return The blue value of color
   */
  unsigned char GetBVal() const {
    return b_;
  }

 private:
  unsigned char a_;
  unsigned char r_;
  unsigned char g_;
  unsigned char b_;
};  // class Color

/**
 * @brief The class for padding data.
 * @details There are left, top, right, bottom padding value.
 * @since_tizen 5.5
 */
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

  /**
   * @brief Copy-constructor.
   * @since_tizen 5.5
   * @param[in] c The object to copy
   */
  Padding(const Padding& c) {
    left_ = c.left_;
    top_ = c.top_;
    right_ = c.right_;
    bottom_ = c.bottom_;
  }

  /**
   * @brief Assignment.
   * @since_tizen 5.5
   * @param[in] c The object to copy
   */
  Padding& operator = (const Padding& c) {
    if (this != &c) {
      left_ = c.left_;
      top_ = c.top_;
      right_ = c.right_;
      bottom_ = c.bottom_;
    }
    return *this;
  }

  /**
   * @brief Move-constructor.
   * @since_tizen 5.5
   * @param[in] c The object to move
   */
  Padding(Padding&& c) noexcept {
    left_ = c.left_;
    c.left_ = 0;

    top_ = c.top_;
    c.top_ = 0;

    right_ = c.right_;
    c.right_ = 0;

    bottom_ = c.bottom_;
    c.bottom_ = 0;
  }

  /**
   * @brief Assignment.
   * @since_tizen 5.5
   * @param[in] c The object to move
   */
  Padding& operator = (Padding&& c) noexcept {
    if (this != &c) {
      left_ = c.left_;
      c.left_ = 0;

      top_ = c.top_;
      c.top_ = 0;

      right_ = c.right_;
      c.right_ = 0;

      bottom_ = c.bottom_;
      c.bottom_ = 0;
    }
    return *this;
  }

  /**
   * @brief Gets left value of padding.
   * @since_tizen 5.5
   * @return The left value of padding
   */
  int GetLeft() const {
    return left_;
  }

  /**
   * @brief Gets top value of padding.
   * @since_tizen 5.5
   * @return The top value of padding
   */
  int GetTop() const {
    return top_;
  }

  /**
   * @brief Gets right value of padding.
   * @since_tizen 5.5
   * @return the right value of padding
   */
  int GetRight() const {
    return right_;
  }

  /**
   * @brief Gets bottom value of padding.
   * @since_tizen 5.5
   * @return The bottom value of padding.
   */
  int GetBottom() const {
    return bottom_;
  }

 private:
  int left_;
  int top_;
  int right_;
  int bottom_;
};  // class  Padding

/**
 * @brief The class for geometry data.
 * @details There are x, y, width, height value.
 * @since_tizen 5.5
 */
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

  /**
   * @brief Copy-constructor.
   * @since_tizen 5.5
   * @param[in] c The object to copy
   */
  Geometry(const Geometry& c) {
    x_ = c.x_;
    y_ = c.y_;
    w_ = c.w_;
    h_ = c.h_;
  }

  /**
   * @brief Assignment.
   * @since_tizen 5.5
   * @param[in] c The object to copy
   */
  Geometry& operator = (const Geometry& c) {
    if (this != &c) {
      x_ = c.x_;
      y_ = c.y_;
      w_ = c.w_;
      h_ = c.h_;
    }
    return *this;
  }

  /**
   * @brief Move-constructor.
   * @since_tizen 5.5
   * @param[in] c The object to move
   */
  Geometry(Geometry&& c) noexcept {
    x_ = c.x_;
    c.x_ = 0;

    y_ = c.y_;
    c.y_ = 0;

    w_ = c.w_;
    c.w_ = 0;

    h_ = c.h_;
    c.h_ = 0;
  }

  /**
   * @brief Assignment.
   * @since_tizen 5.5
   * @param[in] c The object to move
   */
  Geometry& operator = (Geometry&& c) noexcept {
    if (this != &c) {
      x_ = c.x_;
      c.x_ = 0;

      y_ = c.y_;
      c.y_ = 0;

      w_ = c.w_;
      c.w_ = 0;

      h_ = c.h_;
      c.h_ = 0;
    }
    return *this;
  }

  /**
   * @brief Gets x value of geometry.
   * @since_tizen 5.5
   * @return The x value of geometry.
   */
  int GetX() const {
    return x_;
  }

  /**
   * @brief Gets y value of geometry.
   * @since_tizen 5.5
   * @return The y value of geometry.
   */
  int GetY() const {
    return y_;
  }

  /**
   * @brief Gets width value of geometry.
   * @since_tizen 5.5
   * @return The width value of geometry.
   */
  int GetWidth() const {
    return w_;
  }

  /**
   * @brief Gets height value of geometry.
   * @since_tizen 5.5
   * @return The height value of geometry.
   */
  int GetHeight() const {
    return h_;
  }

 private:
  int x_;
  int y_;
  int w_;
  int h_;
};  // class Geometry

/**
 * @brief The class for style data
 * @details The style data consists color, padding, geometry data.
 * @since_tizen 5.5
 */
class EXPORT_API Style {
 public:
  Style() {
  }

  Style(std::shared_ptr<Color> color, std::shared_ptr<Padding> padding,
      std::shared_ptr<Geometry> geometry)
    : color_(color), padding_(padding), geometry_(geometry) {
  }

  Style(std::shared_ptr<Color> color, std::shared_ptr<Padding> padding,
      std::shared_ptr<Geometry> geometry,
      std::shared_ptr<Color> background_color, std::string background_image)
    : color_(color), padding_(padding), geometry_(geometry),
      background_color_(background_color),
      background_image_path_(background_image) {
  }

  virtual ~Style() = default;

  /**
   * @brief Copy-constructor.
   * @since_tizen 5.5
   * @param[in] c The object to copy
   */
  Style(const Style& c) {
    color_ = c.color_;
    padding_ = c.padding_;
    geometry_ = c.geometry_;
    background_color_ = c.background_color_;
    background_image_path_ = c.background_image_path_;
  }

  /**
   * @brief Assignment.
   * @since_tizen 5.5
   * @param[in] c The object to copy
   */
  Style& operator = (const Style& c) {
    if (this != &c) {
      color_ = c.color_;
      padding_ = c.padding_;
      geometry_ = c.geometry_;
      background_color_ = c.background_color_;
      background_image_path_ = c.background_image_path_;
    }
    return *this;
  }

  /**
   * @brief Move-constructor.
   * @since_tizen 5.5
   * @param[in] c The object to move
   */
  Style(Style&& c) noexcept {
    color_ = c.color_;
    c.color_ = nullptr;

    padding_ = c.padding_;
    c.padding_ = nullptr;

    geometry_ = c.geometry_;
    c.geometry_ = nullptr;

    background_color_ = c.background_color_;
    c.background_color_ = nullptr;

    background_image_path_ = c.background_image_path_;
    c.background_image_path_ = "";
  }

  /**
   * @brief Assignment.
   * @since_tizen 5.5
   * @param[in] c The object to move
   */
  Style& operator = (Style&& c) noexcept {
    if (this != &c) {
      color_ = c.color_;
      c.color_ = nullptr;

      padding_ = c.padding_;
      c.padding_ = nullptr;

      geometry_ = c.geometry_;
      c.geometry_ = nullptr;

      background_color_ = c.background_color_;
      c.background_color_ = nullptr;

      background_image_path_ = c.background_image_path_;
      c.background_image_path_ = "";
    }
    return *this;
  }

  /**
   * @brief Gets padding data
   * @since_tizen 5.5
   * @return The padding data
   */
  std::shared_ptr<Padding> GetPadding() const {
    return padding_;
  }

  /**
   * @brief Sets padding data
   * @since_tizen 5.5
   * @param[in] padding The padding data
   */
  void SetPadding(std::shared_ptr<Padding> padding) {
    padding_ = std::move(padding);
  }

  /**
   * @brief Gets color data
   * @since_tizen 5.5
   * @return The color data
   */
  std::shared_ptr<Color> GetColor() const {
    return color_;
  }

  /**
   * @brief Sets color data
   * @since_tizen 5.5
   * @param[in] color The color data
   */
  void SetColor(std::shared_ptr<Color> color) {
    color_ = std::move(color);
  }

  /**
   * @brief Gets geometry data
   * @since_tizen 5.5
   * @return The geometry data
   */
  std::shared_ptr<Geometry> GetGeometry() const {
    return geometry_;
  }

  /**
   * @brief Sets geometry data
   * @since_tizen 5.5
   * @param[in] geometry The geometry data
   */
  void SetGeometry(std::shared_ptr<Geometry> geometry) {
    geometry_ = std::move(geometry);
  }

  /**
   * @brief Gets background image path
   * @since_tizen 5.5
   * @param[in] image_path The background image path
   */
  void SetBackgroundImage(std::string image_path) {
    background_image_path_ = std::move(image_path);
  }

  /**
   * @brief Gets background image path
   * @since_tizen 5.5
   * @return The background image path
   */
  std::string GetBackgroundImage() const {
    return background_image_path_;
  }

  /**
   * @brief Sets background color
   * @since_tizen 5.5
   */
  void SetBackgroundColor(std::shared_ptr<Color> color) {
    background_color_ = std::move(color);
  }

  /**
   * @brief Gets background color
   * @since_tizen 5.5
   * @return The background color
   */
  std::shared_ptr<Color> GetBackgroundColor() const {
    return background_color_;
  }

 private:
  std::shared_ptr<Color> color_;
  std::shared_ptr<Padding> padding_;
  std::shared_ptr<Geometry> geometry_;
  std::shared_ptr<Color> background_color_;
  std::string background_image_path_;
};  // class Style

/**
 * @brief The class for LED data.
 * @details The LED data consists color data and period time.
 * @since_tizen 5.5
 */
class EXPORT_API LEDInfo {
 public:
  LEDInfo() {
  }
  explicit LEDInfo(std::shared_ptr<Color> color)
    : color_(color) {
  }
  virtual ~LEDInfo() = default;

  /**
   * @brief Sets the time period for turning on the LED
   * @since_tizen 5.5
   * @param[in] ms period time
   */
  void SetOnPeriod(int ms) {
    on_period_ = ms;
  }

  /**
   * @brief Gets the time period for turning on the LED
   * @since_tizen 5.5
   * @return The time for turning on the LED
   */
  int GetOnPeriod() const {
    return on_period_;
  }

  /**
   * @brief Sets the time period for turning off the LED
   * @since_tizen 5.5
   * @param[in] ms period time
   */
  void SetOffPeriod(int ms) {
    off_period_ = ms;
  }

  /**
   * @brief Gets the time period for turning off the LED
   * @since_tizen 5.5
   * @return The time for turning off the LED
   */
  int GetOffPeriod() const {
    return off_period_;
  }

  /**
   * @brief Gets the color of LED
   * @since_tizen 5.5
   * @return color data
   */
  std::shared_ptr<Color> GetColor() const {
    return color_;
  }

  void SetColor(std::shared_ptr<Color> color) {
    color_ = std::move(color);
  }

 private:
  std::shared_ptr<Color> color_;
  int on_period_ = 0;
  int off_period_ = 0;
};  // clss LEDInfo

/**
 * @brief The base class for the notification item classes.
 * @details The AbstractItem is abstract class.
 *          The AbstractItem has basic APIs for notification items.
 *          The notification item class have to be a derived class of this class.
 * @since_tizen 5.5
 */
class EXPORT_API AbstractItem {
 public:
  enum Type {
    NullObject,
    Text,
    Image,
    Icon,
    Button,
    ChatMessage,
    CheckBox,
    IconText,
    InputSelector,
    Group,
    Entry,
    Progress,
    Time,
    Custom = 100
  };

  enum Policy {
    None = 0,
    OnBootClear = 1 << 0,
    SimMode = 1 << 1,
  };

  enum MainType {
    MainNone = 0,
    MainTitle,
    MainContents,
    MainIcon,
    MainButton
  };

 public:
  /**
   * @brief Constructor
   * @since_tizen 5.5
   * @param[in] action The AbstractAction for notification item
   */
  AbstractItem(
      std::shared_ptr<AbstractAction> action = std::shared_ptr<AbstractAction>({}));

  /**
   * @brief Constructor
   * @since_tizen 5.5
   * @param[in] id The notification id
   * @param[in] action The AbstractAction for notification item
   */
  AbstractItem(std::string id,
      std::shared_ptr<AbstractAction> action = std::shared_ptr<AbstractAction>({}));

  /**
   * @brief Destructor
   * @since_tizen 5.5
   */
  virtual ~AbstractItem() = 0;

  /**
   * @brief Serialize the data of AbstractItem.
   * @since_tizen 5.5
   * @return Bundle type data
   */
  virtual tizen_base::Bundle Serialize() const = 0;

  /**
   * @brief Deserialize the serialized data.
   * @since_tizen 5.5
   * @param[in] b The serialized Bundle data
   */
  virtual void Deserialize(tizen_base::Bundle b) = 0;

  /**
   * @brief Finds the AbstractItem using by notification item id.
   * @since_tizen 5.5
   * @param[in] id notification item id
   * @return AbstractItem object
   */
  virtual AbstractItem& FindByID(std::string id);

  /**
   * @brief Finds the AbstractItem using by main type.
   * @since_tizen 5.5
   * @param[in] type The main type
   * @return AbstractItem object
   */
  virtual AbstractItem& FindByMainType(MainType type);

  /**
   * @brief Checks the item type exist in this notification.
   * @since_tizen 5.5
   * @param[in] type notification item type
   * @return true if the item type exists
   */
  virtual bool IsItemTypeExist(int type) = 0;

  /**
   * @brief Gets the type of notification item.
   * @since_tizen 5.5
   * @return The type of notification item
   */
  virtual int GetType() const = 0;

  /**
   * @brief Gets the type of notification item from Bundle data.
   * @since_tizen 5.5
   * @return The type of notification item
   */
  static int GetType(tizen_base::Bundle b);

  /**
   * @brief Gets the path of shared file location.
   * @since_tizen 5.5
   * @return The list of shared path.
   */
  virtual std::list<std::string> GetSharedPath() const;

  /**
   * @brief Sets the shared file path to original file path.
   */
  virtual void SetSharedPath();

  /**
   * @brief
   * @since_tizen 5.5
   * @return
   */
  virtual std::list<std::map<std::string, std::string>> GetPathMapList() const;

  /**
   * @brief Gets the notification item id.
   * @since_tizen 5.5
   * @return The notification item id.
   */
  std::string GetId() const;

  /**
   * @brief Sets the notification item id.
   * @since_tizen 5.5
   * @param[in] id notification item id
   */
  void SetId(std::string id);

  std::vector<std::shared_ptr<MultiLanguage>> GetMultiLanguageArr() const;
  std::shared_ptr<MultiLanguage> GetMultiLanguage() const;
  void SetMultiLanguage(std::shared_ptr<MultiLanguage> multi);
  void SetMultiLanguage(std::vector<std::shared_ptr<MultiLanguage>> multi_arr);

  /**
   * @brief Gets AbstractAction for notification item.
   * @since_tizen 5.5
   * @return AbstractAction instance
   */
  std::shared_ptr<AbstractAction> GetAction() const;

  /**
   * @brief Sets AbstractAction for notification item.
   * @since_tizen 5.5
   * @param[in] action AbstractAction instance
   */
  void SetAction(std::shared_ptr<AbstractAction> action);

  /**
   * @brief Sets the style data for notification item.
   * @since_tizen 5.5
   * @return Style instance
   */
  std::shared_ptr<Style> GetStyle() const;

  /**
   * @brief Sets the style data for notification item.
   * @since_tizen 5.5
   * @param[in] style Style instance
   */
  void SetStyle(std::shared_ptr<Style> style);

  /**
   * @brief Sets the visibile state of notification item.
   * @since_tizen 5.5
   * @param[in] visibile The visible state
   */
  void SetVisible(bool visible);

  /**
   * @brief Gets the visibile state of notification item.
   * @since_tizen 5.5
   * @return true if visible, false if not visible.
   */
  bool GetVisible() const;

  /**
   * @brief Sets the enable state of notification item.
   * @since_tizen 5.5
   * @param[in] enable The enable state
   */
  void SetEnable(bool enable);

  /**
   * @brief Gets the enable state of notification item.
   * @since_tizen 5.5
   * @return true if enabled, false if not enabled.
   */
  bool GetEnable() const;

  /**
   * @brief Adds the receiver group for notification item.
   * @since_tizen 5.5
   * @param[in] receiver_group The receiver group for notification item
   */
  void AddReceiver(std::string receiver_group);

  /**
   * @brief Removes the receiver group from the receiver group list.
   * @since_tizen 5.5
   * @param[in] receiver_group The receiver group
   */
  void RemoveReceiver(std::string receiver_group);

  /**
   * @brief Gets the receiver group list.
   * @since_tizen 5.5
   * @return The list of receiver group.
   */
  std::list<std::string> GetReceiverList();

  /**
   * @brief Sets the policy for notification item.
   * @since_tizen 5.5
   * @param[in] policy The policy option
   */
  void SetPolicy(int policy);

  /**
   * @brief Gets the policy for notification item.
   * @since_tizen 5.5
   * @return The policy for notification item.
   */
  int GetPolicy() const;

  /**
   * @brief Gets the channel option for notification item.
   * @since_tizen 5.5
   * @return The channel option for notification item.
   */
  std::string GetChannel() const;

  /**
   * @brief Sets the channel option for notification item.
   * @since_tizen 5.5
   * @param[in] channel The channel option for notification item.
   */
  void SetChannel(std::string channel);

  /**
   * @brief Sets LED option for notification item.
   * @since_tizen 5.5
   * @param[in] led The LEDInfo instance
   */
  void SetLEDInfo(std::shared_ptr<LEDInfo> led);

  /**
   * @brief Gets LED option for notification item.
   * @since_tizen 5.5
   * @return The LEDInfo instance
   */
  std::shared_ptr<LEDInfo> GetLEDInfo() const;

  /**
   * @brief Sets the sound path for notification item.
   * @since_tizen 5.5
   * @param[in] path The sound path
   */
  void SetSoundPath(std::string path);

  /**
   * @brief Sets the vibration path for notification item.
   * @since_tizen 5.5
   * @param[in] path The vibration path
   */
  void SetVibrationPath(std::string path);

  /**
   * @brief Gets the sound path for notification item.
   * @since_tizen 5.5
   * @return The sound path
   */
  std::string GetSoundPath() const;

  /**
   * @brief Gets the vibration path for notification item.
   * @since_tizen 5.5
   * @return The vibration path
   */
  std::string GetVibrationPath() const;

  /**
   * @brief Gets IItemInfo instance to get some information of notification item.
   * @since_tizen 5.5
   * @return The IItemInfo instance
   */
  std::shared_ptr<IItemInfo> GetInfo() const;

  /**
   * @brief Gets the sender app id of notification item.
   * @since_tizen 5.5
   * @return The sender app id.
   */
  std::string GetSenderAppId() const;

  /**
   * @brief Sets the sender app id of notification item.
   * @since_tizen 5.5
   * @param[in] sender_appid The sender app id
   */
  void SetSenderAppId(std::string sender_appid);

  /**
   * @brief Sets the tag of notification item.
   * @since_tizen 5.5
   * @return The tag of notification item
   */
  std::string GetTag() const;

  /**
   * @brief Sets the tag of notification item.
   * @since_tizen 5.5
   * @param[in] tag The tag of notification item
   */
  void SetTag(std::string tag);

  /**
   * @brief Sets the ongoing state of notification item.
   * @since_tizen 5.5
   * @param[in] ongoing The ongoing state of notification item
   */
  void SetOnGoingState(bool ongoing);

  /**
   * @brief Gets the ongoing state of notification item.
   * @since_tizen 5.5
   * @return The ongoing state of notification item
   */
  bool GetOnGoingState() const;

  /**
   * @brief Sets the main type of notification item.
   * @since_tizen 5.5
   * @param[in] target_id The ID of notification item to set
   * @param[in] type The main type
   * @return true if target_id is valid to set
   */
  bool SetMainType(std::string target_id, MainType type);

  /**
   * @brief Gets the main type of notification item.
   * @since_tizen 5.5
   * @return The main type
   */
  MainType GetMainType() const;

  /**
   * @brief Gets the extension data.
   * @since_tizen 5.5
   * @param[in] key
   * @return Bundle
   */
  tizen_base::Bundle GetExtensionData(std::string key);

  /**
   * @brief Sets the extension data.
   * @since_tizen 5.5
   * @param[in] key key string
   * @param[in] value Bundle
   */
  void SetExtensionData(std::string key, tizen_base::Bundle value);

 private:
  class Impl;
  std::unique_ptr<Impl> impl_;
  void UpdateSoundPrivatePath();
  void UpdateVibrationPrivatePath();
};  // class AbstractItem
/* LCOV_EXCL_STOP */

}  // namespace item
}  // namespace notification

#endif  // NOTIFICATION_EX_ABSTRACT_ITEM_H_

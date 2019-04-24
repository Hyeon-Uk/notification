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

#ifndef NOTIFICATION_EX_ABSTRACT_ACTION_H_
#define NOTIFICATION_EX_ABSTRACT_ACTION_H_

#include <memory>
#include <string>

#include <bundle_cpp.h>

#ifndef EXPORT_API
#define EXPORT_API __attribute__((visibility("default")))
#endif

namespace notification {
namespace item {

class AbstractItem;

/**
 * @brief The base class for the notification action classes.
 * @details The AbstractAction is abstract class.
 *          The AbstractAction has basic APIs for notification actions.
 *          The notification action class have to be a derived class of this class.
 * @since_tizen 5.5
 */
class EXPORT_API AbstractAction {
 public:
  enum Type {
    NullObject,
    AppControl,
    Visibility,
    Custom = 100
  };

 public:
  /**
   * @brief Constructor
   * @since_tizen 5.5
   * @param[in] isLoacal
   */
  AbstractAction(bool isLocal);

  /**
   * @brief Constructor
   * @since_tizen 5.5
   * @param[in] isLocal
   * @param[in] extra
   */
  AbstractAction(bool isLocal, std::string extra);

  /**
   * @brief Destructor
   * @since_tizen 5.5
   */
  virtual ~AbstractAction();

  /**
   * @brief Gets the type of action
   * @since_tizen 5.5
   * @return The type of action
   */
  virtual int GetType() const = 0;

  /**
   * @brief Gets the type of action from Bundle data
   * @since_tizen 5.5
   * @param[in] b Bundle type data
   * @return The type of action
   */
  static int GetType(tizen_base::Bundle b);

  /**
   * @brief Serialize the data of AbstractAction.
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
   * @brief Gets whether local or not.
   * @since_tizen 5.5
   * @return true if local, or false
   */
  virtual bool IsLocal() const = 0;

  /**
   * @brief Execute the action
   * @since_tizen 5.5
   * @param[in] item The AbstractItem
   */
  virtual void Execute(std::shared_ptr<AbstractItem> item) = 0;

  /**
   * @brief Gets the extra data
   * @since_tizen 5.5
   * @return The extra data
   */
  virtual std::string GetExtra() const = 0;

 private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace item
}  // namespace notification

#endif  // NOTIFICATION_EX_ABSTRACT_ACTION_H_

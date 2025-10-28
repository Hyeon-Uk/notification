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

#ifndef NOTIFICATION_EX_VISIBILITY_ACTION_H_
#define NOTIFICATION_EX_VISIBILITY_ACTION_H_

#include <string>

#include "notification-ex/abstract_action.h"

namespace notification {
namespace item {

/**
 * @brief The class for VisibilityAction type action.
 * @details The action for visibility of notification.
 * @since_tizen 5.5
 */
class EXPORT_API VisibilityAction  : public AbstractAction {
 public:
  /**
   * @brief Constructor
   * @since_tizen 5.5
   */
  VisibilityAction();

  /**
   * @brief Constructor
   * @since_tizen 5.5
   * @param[in] extra
   */
  VisibilityAction(std::string extra);

  /**
   * @brief Destructor
   * @since_tizen 5.5
   */
  virtual ~VisibilityAction();

  /**
   * @brief Gets the type of action
   * @since_tizen 5.5
   * @return The type of action
   */
  int GetType() const override;

  /**
   * @brief Serialize the data of AbstractAction.
   * @since_tizen 5.5
   * @return Bundle type data
   */
  tizen_base::Bundle Serialize() const override;

  /**
   * @brief Deserialize the serialized data.
   * @since_tizen 5.5
   * @param[in] b The serialized Bundle data
   */
  void Deserialize(tizen_base::Bundle b) override;

  /**
   * @brief Gets whether local or not.
   * @since_tizen 5.5
   * @return true if local, or false
   */
  bool IsLocal() const override;

  /**
   * @brief Execute the action
   * @since_tizen 5.5
   * @param[in] item The AbstractItem
   */
  void Execute(std::shared_ptr<AbstractItem> item) override;

  /**
   * @brief Gets the extra data
   * @since_tizen 5.5
   * @return The extra data
   */
  std::string GetExtra() const override;

  /**
   * @brief Sets the visibility
   * @since_tizen 5.5
   * @param[in] id
   */
  void SetVisibility(std::string id, bool visible);

 private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace item
}  // namespace notification

#endif  // NOTIFICATION_EX_VISIBILITY_ACTION_H_

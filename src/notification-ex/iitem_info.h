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

#ifndef NOTIFICATION_EX_IITEM_INFO_H_
#define NOTIFICATION_EX_IITEM_INFO_H_

#include <time.h>

namespace notification {
namespace item {

/* LCOV_EXCL_START */
/**
 * @brief The interface class for ItemInfo
 * @since_tizen 5.5
 */
class EXPORT_API IItemInfo {
 public:
  /**
   * @brief Destructor
   * @since_tizen 5.5
   */
  virtual ~IItemInfo() = default;

  /**
   * @brief Gets hide time of notification
   * @since_tizen 5.5
   * @return The hide time value
   */
  virtual int GetHideTime() const = 0;

  /**
   * @brief Sets hide time of notification
   * @since_tizen 5.5
   * @param[in] hide_time The hide time
   */
  virtual void SetHideTime(int hide_time) = 0;

  /**
   * @brief Gets delete time of notification
   * @since_tizen 5.5
   * @return The delete time value
   */
  virtual int GetDeleteTime() const = 0;

  /**
   * @brief Sets delete time of notification
   * @since_tizen 5.5
   * @param[in] delete_time The delete time
   */
  virtual void SetDeleteTime(int delete_time)  = 0;

  /**
   * @brief Gets time information
   * @since_tizen 5.5
   * @return The time information
   */
  virtual time_t GetTime() const = 0;
};
/* LCOV_EXCL_STOP */

}  // namespace item
}  // namespace notification
#endif  // NOTIFICATION_EX_IITEM_INFO_H_

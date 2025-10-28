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

#ifndef NOTIFICATION_EX_MULTI_LANGUAGE_H_
#define NOTIFICATION_EX_MULTI_LANGUAGE_H_

#include <string>
#include <memory>
#include <vector>

#include <bundle_cpp.h>

#ifndef EXPORT_API
#define EXPORT_API __attribute__((visibility("default")))
#endif

namespace notification {
namespace item {

/* LCOV_EXCL_START */
/**
 * @brief The class for MultiLanguage type notification.
 * @details The class to make the notification with multi language.
 * @since_tizen 5.5
 */
class EXPORT_API MultiLanguage {
 public:
  MultiLanguage(std::string msgid, std::string format,
      std::vector<std::string> args);
  MultiLanguage(std::string serialized);
  virtual ~MultiLanguage();
  tizen_base::Bundle Serialize() const;
  void Deserialize(tizen_base::Bundle b);
  void UpdateString(std::string domain, std::string locale_directory);
  void UpdateString();
  std::string GetTranslatedString();

 private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};  // class MultiLanguage
/* LCOV_EXCL_STOP */

}  // namespace item
}  // namespace notification

#endif  // NOTIFICATION_EX_MULTI_LANGUAGE_H_

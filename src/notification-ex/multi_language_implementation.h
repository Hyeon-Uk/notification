

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

#ifndef NOTIFICATION_EX_MULTI_LANGUAGE_IMPLEMENTATION_H_
#define NOTIFICATION_EX_MULTI_LANGUAGE_IMPLEMENTATION_H_

#include <string>
#include <memory>
#include <vector>

#include "notification-ex/multi_language.h"

namespace notification {
namespace item {

/* LCOV_EXCL_START */
class MultiLanguage::Impl {
 public:
  virtual ~Impl();

 private:
  Impl(std::string msgid, std::string format, std::vector<std::string> args,
    MultiLanguage* parent);
  Impl(std::string msgid, std::string format, MultiLanguage* parent);

 private:
  friend class MultiLanguage;

  std::string msgid_;
  std::string format_;
  std::vector<std::string> args_;
  std::string translated_;
  std::string domain_name_;
  std::string locale_directory_;
  MultiLanguage* parent_;
};
/* LCOV_EXCL_STOP */

}  // namespace item
}  // namespace notification

#endif  // NOTIFICATION_EX_MULTI_LANGUAGE_IMPLEMENTATION_H_

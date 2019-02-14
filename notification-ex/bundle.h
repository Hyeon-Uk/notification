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

#ifndef NOTIFICATION_EX_BUNDLE_H_
#define NOTIFICATION_EX_BUNDLE_H_

#include <bundle.h>

#include <string>

#ifndef EXPORT_API
#define EXPORT_API __attribute__((visibility("default")))
#endif

namespace notification {
namespace item {

class EXPORT_API Bundle {
 public:
  Bundle();
  explicit Bundle(std::string raw);
  explicit Bundle(bundle* raw);
  ~Bundle();

  Bundle(const Bundle&);
  Bundle& operator=(const Bundle&);

  Bundle(Bundle && other);
  Bundle& operator=(Bundle && other);

  const bundle* GetConstRaw() const;
  bundle* GetRaw();
  const char* ToString();

 private:
  bundle* raw_ = NULL;
  char* str_raw_ = NULL;
};

}  // namespace item
}  // namespace notification

#endif  // NOTIFICATION_EX_BUNDLE_H_

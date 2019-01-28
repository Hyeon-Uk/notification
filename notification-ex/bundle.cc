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

#include "notification-ex/bundle.h"
#include "notification-ex/exception.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "NOTIFICATION_EX"

namespace notification {
namespace item {

Bundle::Bundle() {
	raw_ = bundle_create();
}

const char* Bundle::ToString() {
  int str_len = 0;
  bundle_raw* temp;

  if (str_raw_ != NULL)
    return str_raw_;

  bundle_encode(raw_, &temp, &str_len);

  str_raw_ = reinterpret_cast<char*>(temp);
  return str_raw_;
}

Bundle::~Bundle() {
  if (raw_) {
    bundle_free(raw_);
    raw_ = NULL;
  }
  if (str_raw_) {
    free(str_raw_);
    str_raw_ = NULL;
  }
}

} //namespace item
} //namespace notification

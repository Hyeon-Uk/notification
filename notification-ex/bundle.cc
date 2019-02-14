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
  if (raw_ == NULL)
    THROW(NOTIFICATION_ERROR_OUT_OF_MEMORY);
}

Bundle::Bundle(std::string raw) {
  if (!raw.empty()) {
    raw_ = bundle_decode(reinterpret_cast<const bundle_raw*>(raw.c_str()),
                         raw.length());
    if (raw_ == NULL) {
      int ret = get_last_result();
      if (ret == BUNDLE_ERROR_OUT_OF_MEMORY)
        THROW(NOTIFICATION_ERROR_OUT_OF_MEMORY);
      else
        THROW(NOTIFICATION_ERROR_INVALID_PARAMETER);
    }
  } else {
    raw_ = bundle_create();
    if (raw_ == NULL)
      THROW(NOTIFICATION_ERROR_OUT_OF_MEMORY);
  }
}

Bundle::Bundle(bundle* raw) {
  raw_ = bundle_dup(raw);
  if (raw_ == NULL)
    THROW(NOTIFICATION_ERROR_OUT_OF_MEMORY);
}

Bundle::Bundle(Bundle&& b) : raw_(b.raw_) {
  b.raw_ = nullptr;
}

Bundle& Bundle::operator = (Bundle&& b) {
  raw_ = b.raw_;
  b.raw_ = nullptr;
  return *this;
}

Bundle::Bundle(const Bundle& b) : raw_(bundle_dup(b.raw_)) {
}

Bundle& Bundle::operator = (const Bundle& b) {
  raw_ = bundle_dup(b.raw_);
  if (raw_ == NULL)
    THROW(NOTIFICATION_ERROR_OUT_OF_MEMORY);
  return *this;
}

const bundle* Bundle::GetConstRaw() const {
  return raw_;
}

bundle* Bundle::GetRaw() {
  return raw_;
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

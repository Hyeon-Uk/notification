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

#ifndef NOTIFICATION_EX_EXCEPTION_H_
#define NOTIFICATION_EX_EXCEPTION_H_

#include <dlog.h>

#include <string>
#include <exception>

#include "notification-ex/common.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "NOTIFICATION_EX"

#define THROW(error_code) throw Exception(error_code, __FILE__, __LINE__)

namespace notification {

/* LCOV_EXCL_START */
class Exception : public std::exception {
 public:
  explicit Exception(int error_code, std::string file = __FILE__,
      int line = __LINE__ ) {
    error_code_ = error_code;
    message_ = file.substr(file.find_last_of("/") + 1) + ":"
        + std::to_string(line) + GetErrorString(error_code);
    LOGE("%s", message_.c_str());
  }
  virtual ~Exception() {}
  virtual const char *what(void) const noexcept {
    return message_.c_str();
  }
  int GetErrorCode() {
    return error_code_;
  }

 private:
  int error_code_;
  std::string message_;
  std::string GetErrorString(int error_code) {
    switch (error_code) {
    case ERROR_INVALID_PARAMETER:
      return ": INVALID_PARAMETER";
    case ERROR_OUT_OF_MEMORY:
      return ": OUT_OF_MEMORY";
    case ERROR_IO_ERROR:
      return ": IO_ERROR";
    case ERROR_PERMISSION_DENIED:
      return ": PERMISSION_DENIED";
    case ERROR_FROM_DB:
      return ": ERROR_FROM_DB";
    case ERROR_ALREADY_EXIST_ID:
      return ": ALREADY_EXIST_ID";
    case ERROR_FROM_DBUS:
      return ": ERROR_FROM_DBUS";
    case ERROR_NOT_EXIST_ID:
      return ": NOT_EXIST_ID";
    case ERROR_SERVICE_NOT_READY:
      return ": SERVICE_NOT_READY";
    default:
      return "";
    }
  }
};  // class Exception
/* LCOV_EXCL_STOP */

}  // namespace notification

#endif  // NOTIFICATION_EX_EXCEPTION_H_

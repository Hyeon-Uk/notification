/*
 * Copyright (c) 2020 Samsung Electronics Co., Ltd.
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

#ifndef NOTIFICATION_EX_SOCKET_HANDLER_H_
#define NOTIFICATION_EX_SOCKET_HANDLER_H_

#include <unistd.h>

namespace notification {

/* LCOV_EXCL_START */
class SocketHandler {
 public:
  SocketHandler(int fd):fd_(fd){}
  ~SocketHandler() {
    if(fd_!=-1)
      close(fd_);
  }
  operator int() const { return fd_; }
  int Get() {
    return fd_;
  }

 private:
  int fd_ = -1;
};
/* LCOV_EXCL_STOP */

}  // namespace notification
#endif  // NOTIFICATION_EX_SOCKET_HANDLER_H_
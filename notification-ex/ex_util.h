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

#ifndef NOTIFICATION_EX_EX_UTIL_H_
#define NOTIFICATION_EX_EX_UTIL_H_

#include <gmodule.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <bundle_cpp.h>

#include <string>
#include <list>

namespace notification {
namespace util {
  std::string GetAppId();
  std::string GetAppId(pid_t pid);
  int GetRequestId();
  std::string GetPkgId();
  std::string GetDomainName();
  std::string GetLocaleDirectory();
  unsigned int GetSocketBufferSize(int fd, int optname);
  unsigned int ReadSocket(int fd, char* buffer, unsigned int nbytes);
  int WriteSocket(int fd, const char* buffer, unsigned int nbytes);
  std::list<tizen_base::Bundle> ReadBundleList(int recvfd, int noti_cnt);
  int ConvertDBusError(int error);
}  // namespace util
}  // namespace notification

#endif  // NOTIFICATION_EX_EX_UTIL_H_

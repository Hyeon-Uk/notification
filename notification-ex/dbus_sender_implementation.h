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

#ifndef NOTIFICATION_EX_DBUS_SENDER_IMPLEMENTATION_H_
#define NOTIFICATION_EX_DBUS_SENDER_IMPLEMENTATION_H_

#include <string>
#include <memory>
#include <list>

#include "notification-ex/dbus_sender.h"

namespace notification {

class DBusSender::Impl {
 public:
  virtual ~Impl();

 private:
  friend class DBusSender;
  Impl(DBusSender* parent, std::string path);

 private:
  bool EmitSignal(std::string bus_name, std::string signal_name, GVariant* data);
  GDBusMessage* MethodCall(std::string appid, std::string method_name, Bundle serialized);
  std::string path_;
  DBusSender* parent_;
};

}  // namespace notification
#endif  // NOTIFICATION_EX_DBUS_SENDER_IMPLEMENTATION_H_
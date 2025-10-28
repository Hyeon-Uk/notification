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

#ifndef NOTIFICATION_EX_DBUS_SENDER_H_
#define NOTIFICATION_EX_DBUS_SENDER_H_

#include <bundle_cpp.h>

#include <string>
#include <list>

#include "notification-ex/event_sender_interface.h"
#include "notification-ex/ievent_info.h"

#ifndef EXPORT_API
#define EXPORT_API __attribute__((visibility("default")))
#endif

namespace notification {

class EXPORT_API DBusSender : public IEventSender {
 public:
  DBusSender(std::string path);
  virtual ~DBusSender();
  void Notify(const IEventInfo& info, std::list<tizen_base::Bundle> serialized,
      std::string dest_appid = "") override;
  void SendMessage(const IEventInfo& info, std::list<tizen_base::Bundle> serialized,
      std::string dest_appid = "") override;
  std::list<tizen_base::Bundle> Request(const IEventInfo& info) override;
  int RequestReturnValue(const IEventInfo& info) override;

 private:
  void Notify(GVariantBuilder* builder,
        const IEventInfo& info, std::string dest_appid);

 private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace notification

#endif  // NOTIFICATION_EX_DBUS_SENDER_H_

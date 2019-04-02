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

#ifndef NOTIFICATION_EX_MOCK_SENDER_H_
#define NOTIFICATION_EX_MOCK_SENDER_H_

#include "notification-ex/ex_bundle.h"

#ifndef EXPORT_API
#define EXPORT_API __attribute__((visibility("default")))
#endif

namespace notification {

class EXPORT_API MockSender : public IEventSender {
 public:
  MockSender();
  virtual ~MockSender();

  void Notify(tizen_base::Bundle serialized) override;
};

}  // namespace notification

#endif  // NOTIFICATION_EX_MOCK_SENDER_H_

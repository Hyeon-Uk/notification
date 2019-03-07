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

#ifndef NOTIFICATION_EX_ABSTRACT_ACTION_H_
#define NOTIFICATION_EX_ABSTRACT_ACTION_H_

#include <memory>
#include <string>

#include "notification-ex/ex_bundle.h"

#ifndef EXPORT_API
#define EXPORT_API __attribute__((visibility("default")))
#endif

namespace notification {
namespace item {

class AbstractItem;
class EXPORT_API AbstractAction {
 public:
  enum Type {
    NullObject,
    AppControl,
    Visibility,
    Custom = 100
  };

 public:
  AbstractAction(bool isLocal);
  AbstractAction(bool isLocal, std::string extra);
  virtual ~AbstractAction();

  virtual int GetType() const = 0;
  static int GetType(Bundle b);
  virtual Bundle Serialize() const = 0;
  virtual void Deserialize(Bundle b) = 0;
  virtual bool IsLocal() const = 0;
  virtual void Execute(std::shared_ptr<AbstractItem> item) = 0;
  virtual std::string GetExtra() const = 0;

 private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace item
}  // namespace notification

#endif  // NOTIFICATION_EX_ABSTRACT_ACTION_H_

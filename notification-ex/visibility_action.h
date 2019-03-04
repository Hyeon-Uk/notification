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

#ifndef NOTIFICATION_EX_VISIBILITY_ACTION_H_
#define NOTIFICATION_EX_VISIBILITY_ACTION_H_

#include "notification-ex/abstract_action.h"

namespace notification {
namespace item {

class EXPORT_API VisibilityAction  : public AbstractAction {
 public:
  VisibilityAction();
  VisibilityAction(std::string extra);
  virtual ~VisibilityAction();

  Bundle Serialize() override;
  void Deserialize(Bundle b) override;
  bool IsLocal() const override;
  void Execute(std::shared_ptr<AbstractItem> item) override;
  std::string GetExtra() const override;
  void SetVisibility(std::string id, bool visible);

 private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace item
}  // namespace notification

#endif  // NOTIFICATION_EX_VISIBILITY_ACTION_H_

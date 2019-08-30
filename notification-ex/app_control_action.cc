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

#include <dlog.h>
#include <app_control_internal.h>

#include <string>

#include "notification-ex/app_control_action.h"
#include "notification-ex/app_control_action_implementation.h"
#include "notification-ex/exception.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "NOTIFICATION_EX"
#define APPCONTROL_ACTION_KEY "__APPCONTROL_ACTION_KEY__"

using namespace tizen_base;

namespace notification {
namespace item {

AppControlAction::AppControlAction(app_control_h app_control)
    : AbstractAction(true), impl_(new Impl(this)) {
  app_control_h control_;
  app_control_clone(&control_, app_control);
  impl_->control_ = control_;
}

AppControlAction::AppControlAction(app_control_h app_control, std::string extra)
    : AbstractAction(true, extra), impl_(new Impl(this)) {
  app_control_h control_;
  app_control_clone(&control_, app_control);
  impl_->control_ = control_;
}

AppControlAction::Impl::Impl(AppControlAction* parent)
    : parent_(parent) {
  LOGI("AppControlAction created");
}

AppControlAction::~AppControlAction() = default;
AppControlAction::Impl::~Impl() {
  if (control_)
    app_control_destroy(control_);
}

int AppControlAction::GetType() const {
  return AbstractAction::AppControl;
}

Bundle AppControlAction::Serialize() const {
  Bundle b;
  bundle* control_b = NULL;
  bundle_raw* control_raw;
  int len = 0;

  app_control_export_as_bundle(impl_->control_, &control_b);
  if (control_b == NULL) {
    LOGE("failed to get bundle from app_control");
    return {};
  }

  bundle_encode(control_b, &control_raw, &len);
  if (len <= 0) {
    LOGE("bundle encode failed");
    bundle_free(control_b);
    return {};
  }

  b = AbstractAction::Serialize();
  b.Add(APPCONTROL_ACTION_KEY,
    std::string(reinterpret_cast<const char*>(control_raw)));

  bundle_free(control_b);
  free(control_raw);
  return b;
}

void AppControlAction::Deserialize(Bundle b) {
  app_control_h app_control = nullptr;
  bundle* b_ = nullptr;
  std::string control_str;
  int ret;

  AbstractAction::Deserialize(b);

  control_str = b.GetString(APPCONTROL_ACTION_KEY);

  b_ = bundle_decode((bundle_raw*)control_str.c_str(), control_str.length());
  if (b_ == nullptr) {
    LOGE("bundle_decode failed");
    return;
  }

  ret = app_control_create(&app_control);
  if (ret != APP_CONTROL_ERROR_NONE) {
    LOGE("failed to create app_control");
    bundle_free(b_);
    return;
  }

  ret = app_control_import_from_bundle(app_control, b_);
  bundle_free(b_);
  if (ret != APP_CONTROL_ERROR_NONE) {
    LOGE("failed to make app_control from bundle");
    app_control_destroy(app_control);
    return;
  }

  impl_->control_ = app_control;
}

bool AppControlAction::IsLocal() const {
  return AbstractAction::IsLocal();
}

void AppControlAction::Execute(std::shared_ptr<AbstractItem> item) {
  app_control_send_launch_request(impl_->control_, NULL, NULL);
}

std::string AppControlAction::GetExtra() const {
  return AbstractAction::GetExtra();
}

void AppControlAction::SetAppControl(app_control_h app_control) {
  if (impl_->control_ != nullptr)
    app_control_destroy(impl_->control_);

  app_control_h control_;
  app_control_clone(&control_, app_control);

  impl_->control_ = control_;
}

app_control_h AppControlAction::GetAppControl() const {
  return impl_->control_;
}

}  // namespace item
}  // namespace notification

/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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

#ifndef UNIT_TESTS_MOCK_APP_CONTROL_MOCK_H_
#define UNIT_TESTS_MOCK_APP_CONTROL_MOCK_H_

#include <gmock/gmock.h>
#include <app_control.h>
#include <app_control_internal.h>
#include <bundle.h>

#include "module_mock.hh"

class AppControlMock : public virtual ModuleMock {
 public:
  virtual ~AppControlMock() {}
  MOCK_METHOD1(app_control_create, int(app_control_h*));
  MOCK_METHOD2(app_control_import_from_bundle, int(app_control_h, bundle*));
  MOCK_METHOD1(app_control_destroy, int(app_control_h));
  MOCK_METHOD2(app_control_export_as_bundle, int(app_control_h, bundle**));
};

int app_control_mock_real_create(app_control_h* h);
int app_control_mock_real_import_from_bundle(app_control_h h, bundle* b);
int app_control_mock_real_destroy(app_control_h h);
int app_control_mock_real_export_as_bundle(app_control_h h, bundle** b);

#endif  // UNIT_TESTS_MOCK_APP_CONTROL_MOCK_H_

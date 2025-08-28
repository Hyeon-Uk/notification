/*
 * Copyright (c) 2022 Samsung Electronics Co., Ltd.
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

#ifndef UNIT_TESTS_MOCK_AUL_MOCK_H_
#define UNIT_TESTS_MOCK_AUL_MOCK_H_

#include <gmock/gmock.h>
#include <aul.h>
#include <aul_app_event.h>
#include <aul_app_group.h>
#include <aul_app_manager.h>
#include <aul_svc.h>
#include <aul_window.h>

#include "module_mock.hh"

class AulMock : public virtual ModuleMock {
 public:
  virtual ~AulMock() {}

  MOCK_METHOD0(aul_getuid, uid_t());
  MOCK_METHOD3(aul_app_get_appid_bypid, int(int, char*, int));
  MOCK_METHOD3(aul_app_get_pkgid_bypid, int(int, char*, int));
};

#endif  // UNIT_TESTS_MOCK_AUL_MOCK_H_

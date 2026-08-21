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

#include "app_control_mock.hh"

#include <dlfcn.h>

#include "test_fixture.hh"

/* ---------- real-function wrappers (RTLD_NEXT from this translation unit) */

int app_control_mock_real_create(app_control_h* h) {
  static auto fn =
      reinterpret_cast<int (*)(app_control_h*)>(
          dlsym(RTLD_NEXT, "app_control_create"));
  return fn ? fn(h) : APP_CONTROL_ERROR_OUT_OF_MEMORY;
}

int app_control_mock_real_import_from_bundle(app_control_h h, bundle* b) {
  static auto fn =
      reinterpret_cast<int (*)(app_control_h, bundle*)>(
          dlsym(RTLD_NEXT, "app_control_import_from_bundle"));
  return fn ? fn(h, b) : APP_CONTROL_ERROR_IO_ERROR;
}

int app_control_mock_real_destroy(app_control_h h) {
  static auto fn =
      reinterpret_cast<int (*)(app_control_h)>(
          dlsym(RTLD_NEXT, "app_control_destroy"));
  return fn ? fn(h) : APP_CONTROL_ERROR_NONE;
}

int app_control_mock_real_export_as_bundle(app_control_h h, bundle** b) {
  static auto fn =
      reinterpret_cast<int (*)(app_control_h, bundle**)>(
          dlsym(RTLD_NEXT, "app_control_export_as_bundle"));
  return fn ? fn(h, b) : APP_CONTROL_ERROR_INVALID_PARAMETER;
}


extern "C" int app_control_create(app_control_h* app_control) {
  auto* m = dynamic_cast<AppControlMock*>(TestFixture::mock_.get());
  if (m) return m->app_control_create(app_control);
  return app_control_mock_real_create(app_control);
}

extern "C" int app_control_import_from_bundle(app_control_h app_control,
                                               bundle* data) {
  auto* m = dynamic_cast<AppControlMock*>(TestFixture::mock_.get());
  if (m) return m->app_control_import_from_bundle(app_control, data);
  return app_control_mock_real_import_from_bundle(app_control, data);
}

extern "C" int app_control_destroy(app_control_h app_control) {
  auto* m = dynamic_cast<AppControlMock*>(TestFixture::mock_.get());
  if (m) return m->app_control_destroy(app_control);
  return app_control_mock_real_destroy(app_control);
}

extern "C" int app_control_export_as_bundle(app_control_h app_control,
                                             bundle** data) {
  auto* m = dynamic_cast<AppControlMock*>(TestFixture::mock_.get());
  if (m) return m->app_control_export_as_bundle(app_control, data);
  return app_control_mock_real_export_as_bundle(app_control, data);
}

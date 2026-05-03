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

#ifndef NOTIFICATION_RAII_BUNDLE_HANDLES_HH_
#define NOTIFICATION_RAII_BUNDLE_HANDLES_HH_

#include <memory>
#include <bundle.h>
#include <bundle_internal.h>

namespace notification::raii {

/* Phase 3 RAII for the encoded raw bundle output of `bundle_encode`,
 * which the .c-style code stored in `char *bX = NULL;` slots and
 * later released via `bundle_free_encoded_rawdata((bundle_raw **)&bX)`.
 *
 * Used by notification_noti.cc, notification_internal.cc,
 * notification_ipc.cc.
 *
 * The deleter accepts a `char *` (matching the .c-style storage
 * type) and casts to `bundle_raw **` for the release call, mirroring
 * what `__free_encoded_data` did in noti.cc. */
struct BundleEncodedDeleter {
  void operator()(char *p) const noexcept {
    if (p != nullptr) {
      bundle_raw *r = reinterpret_cast<bundle_raw *>(p);
      bundle_free_encoded_rawdata(&r);
    }
  }
};
using BundleEncodedPtr = std::unique_ptr<char, BundleEncodedDeleter>;

/* Phase 3 RAII for `bundle *` returned by bundle_create / bundle_dup /
 * app_control_export_as_bundle / _create_bundle_from_bundle_raw,
 * released by `bundle_free`. */
struct BundleDeleter {
  void operator()(bundle *b) const noexcept {
    if (b != nullptr) bundle_free(b);
  }
};
using BundlePtr = std::unique_ptr<bundle, BundleDeleter>;

}  // namespace notification::raii

#endif  // NOTIFICATION_RAII_BUNDLE_HANDLES_HH_

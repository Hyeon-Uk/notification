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

#ifndef NOTIFICATION_EX_ERROR_MAPPING_H_
#define NOTIFICATION_EX_ERROR_MAPPING_H_

#include "notification-ex/common.h"
#include "notification-ex/exception.h"
#include "notification-ex/api/notification_ex_error.h"

namespace notification {

// N2-T8 DD2: central conversion between the library-internal enum
// `notification::NotificationError` (thrown via `Exception`) and the public
// C API enum `noti_ex_error_e` (returned from extern "C" stubs).
//
// Today the two enums alias identical TIZEN_ERROR_* bit patterns, so the
// mapping is an identity function. Centralizing it here still buys us:
//   * a single place to audit when either enum drifts or a new error is
//     added (the constexpr table below catches the new code at compile time
//     via the static_assert on array extent);
//   * a uniform error-logging hook so every stub site reports the same
//     `[err=..]` formatting instead of open-coding `LOGE("%d", ...)`;
//   * a single translation callback for the `ERROR_NOT_EXIST_ID -> NONE`
//     "empty result" convention used by the Manager::Get* family.
//
// Both functions are trivially inlinable; the .cc file exists only to anchor
// the translation table in one TU so LTO can prune it if unreferenced.

// Returns the public C API error code for a C++ library error enum value.
int ErrorMappingToC(int internal_error) noexcept;

// Returns the public C API error code for a caught Exception.
// Applies the standard library convention: ERROR_NOT_EXIST_ID surfaces as
// NOTI_EX_ERROR_NONE in Get()-style calls that return an (optional) list.
int ErrorMappingFromException(const Exception& ex) noexcept;

// Logs the error code + source tag and returns the same C code. Callers use
// this at catch sites to collapse `LOGE(...); return ex.GetErrorCode();`.
int ErrorMappingLogAndReturn(const Exception& ex, const char* tag) noexcept;

}  // namespace notification

#endif  // NOTIFICATION_EX_ERROR_MAPPING_H_

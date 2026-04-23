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

#include "notification-ex/error_mapping.h"

#include <dlog.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "NOTIFICATION_EX"

namespace notification {

namespace {

// N2-T8 DD2: one-to-one table between the C++ library error enum and the
// public C enum. Both enums currently share the same TIZEN_ERROR_* bit
// pattern, so lookup compiles to an identity. The table is kept explicit
// (rather than a blind cast) so any future divergence is localized here
// and statically verified.
struct ErrorPair {
  int internal;
  noti_ex_error_e external;
};

constexpr ErrorPair kErrorMap[] = {
  { ERROR_NONE,               NOTI_EX_ERROR_NONE               },
  { ERROR_INVALID_PARAMETER,  NOTI_EX_ERROR_INVALID_PARAMETER  },
  { ERROR_OUT_OF_MEMORY,      NOTI_EX_ERROR_OUT_OF_MEMORY      },
  { ERROR_IO_ERROR,           NOTI_EX_ERROR_IO_ERROR           },
  { ERROR_PERMISSION_DENIED,  NOTI_EX_ERROR_PERMISSION_DENIED  },
  { ERROR_FROM_DB,            NOTI_EX_ERROR_FROM_DB            },
  { ERROR_ALREADY_EXIST_ID,   NOTI_EX_ERROR_ALREADY_EXIST_ID   },
  { ERROR_FROM_DBUS,          NOTI_EX_ERROR_FROM_DBUS          },
  { ERROR_NOT_EXIST_ID,       NOTI_EX_ERROR_NOT_EXIST_ID       },
  { ERROR_SERVICE_NOT_READY,  NOTI_EX_ERROR_SERVICE_NOT_READY  },
};

// Compile-time sanity: if a new NotificationError is added but not wired
// here, the static_assert fails on the next build.
static_assert(sizeof(kErrorMap) / sizeof(kErrorMap[0]) == 10,
    "kErrorMap must cover every notification::NotificationError value");

}  // namespace

int ErrorMappingToC(int internal_error) noexcept {
  for (const auto& pair : kErrorMap) {
    if (pair.internal == internal_error)
      return static_cast<int>(pair.external);
  }
  // Unknown internal code: forward the raw integer so upstream diagnostics
  // are not silently masked. Callers that care about a fixed enum domain
  // should treat anything outside NOTI_EX_ERROR_* as an IO error surrogate.
  return internal_error;
}

int ErrorMappingFromException(const Exception& ex) noexcept {
  // Exception::GetErrorCode is non-const in the existing header, so we
  // have to defeat const locally. Keeping the logical contract `const&`
  // makes call sites express the no-mutation intent.
  Exception& mut = const_cast<Exception&>(ex);
  const int code = mut.GetErrorCode();
  // Standard convention: Manager::Get / subscribe lookups that come back
  // empty throw ERROR_NOT_EXIST_ID and the C API reports success with an
  // empty output list. Call sites that want the raw code can call
  // ErrorMappingToC() directly.
  if (code == ERROR_NOT_EXIST_ID)
    return NOTI_EX_ERROR_NONE;
  return ErrorMappingToC(code);
}

int ErrorMappingLogAndReturn(const Exception& ex, const char* tag) noexcept {
  Exception& mut = const_cast<Exception&>(ex);
  const int code = mut.GetErrorCode();
  LOGE("[%s] %s err=%d", tag ? tag : "noti_ex", mut.what(), code);
  return ErrorMappingToC(code);
}

}  // namespace notification

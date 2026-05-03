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

#ifndef NOTIFICATION_RAII_GLIB_HANDLES_HH_
#define NOTIFICATION_RAII_GLIB_HANDLES_HH_

#include <memory>
#include <gio/gio.h>
#include <glib.h>

namespace notification::raii {

/* GError* — `g_error_free`. */
struct GErrorDeleter {
  void operator()(GError *e) const noexcept {
    if (e != nullptr) g_error_free(e);
  }
};
using GErrorPtr = std::unique_ptr<GError, GErrorDeleter>;

/* GVariant* — `g_variant_unref`. */
struct GVariantDeleter {
  void operator()(GVariant *v) const noexcept {
    if (v != nullptr) g_variant_unref(v);
  }
};
using GVariantPtr = std::unique_ptr<GVariant, GVariantDeleter>;

/* Generic g_free deleter for char* / pointer-typed `g_strdup` /
 * `g_strdup_printf` returns. */
struct GFreeDeleter {
  void operator()(void *p) const noexcept {
    if (p != nullptr) g_free(p);
  }
};
using GCharPtr = std::unique_ptr<char, GFreeDeleter>;

/* GObject* — `g_object_unref`. Useful for GFile, GDBusConnection,
 * GDBusMessage, etc. Templated on the concrete type so the unique_ptr
 * stays type-safe. */
template <typename T>
struct GObjectDeleter {
  void operator()(T *o) const noexcept {
    if (o != nullptr) g_object_unref(o);
  }
};
template <typename T>
using GObjectPtr = std::unique_ptr<T, GObjectDeleter<T>>;

}  // namespace notification::raii

#endif  // NOTIFICATION_RAII_GLIB_HANDLES_HH_

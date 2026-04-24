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

#ifndef NOTIFICATION_EX_EVENT_INFO_INTERNAL_H_
#define NOTIFICATION_EX_EVENT_INFO_INTERNAL_H_

#include <bundle_cpp.h>
#include <glib.h>

#include <string>
#include <list>

#include "notification-ex/common.h"
#include "notification-ex/ievent_info_internal.h"

#ifndef EXPORT_API
#define EXPORT_API __attribute__((visibility("default")))
#endif

namespace notification {

class EXPORT_API EventInfo : public IEventInfoInternal {
 public:
  EventInfo(int type, std::string owner, std::string channel = "",
      std::string item_id = "");
  EventInfo(tizen_base::Bundle serialized);
  virtual ~EventInfo();
  uid_t GetUid() const override;
  void SetUid(uid_t uid) override;
  NotificationError GetError() const override;
  void SetError(NotificationError error) override;
  int GetEventType() const override;
  void SetEventType(int type) override;
  std::string GetOwner() const override;
  uid_t GetValidatedUid() const override;
  void SetValidatedUid(uid_t uid) override;
  void SetValidatedOwner(std::string owner) override;
  std::string GetValidatedOwner() const override;
  std::string GetChannel() const override;
  std::string GetItemId() const override;
  int GetRequestId() const override;
  tizen_base::Bundle Serialize() const override;
  static std::string GetString(int type);

  /*
   * N3-T2 C2 (single-field direct GVariant path):
   *
   * Phase 3 scope is intentionally narrow — we bypass the
   * `Serialize() -> Bundle -> bundle_encode() -> g_variant_new("ss…")`
   * double-encoding pipeline only for the four short fields that the
   * wire-level IEventInfo section actually pins at the top of every
   * D-Bus payload:
   *
   *   - "type"       : int32   (EventType enum, also chosen for
   *                             signal name)
   *   - "uid"        : int32   (uid_t widened)
   *   - "request_id" : int32
   *   - "error"      : int32   (NotificationError enum)
   *
   * `ToGVariant()` returns a floating-ref GVariant of type `a{sv}` with
   * exactly these four entries. `FromGVariant()` consumes an `a{sv}`
   * and repopulates the four fields; unknown keys are ignored so
   * future additions are forward-compatible. The remaining fields
   * (owner / channel / item_id / validated_uid / validated_owner) and
   * the AbstractItem payload continue to flow through the existing
   * bundle path for Phase 3; Phase 4 will extend this direct path to
   * the rest of the wire format.
   *
   * Returned GVariant has a floating ref (per g_variant_builder_end
   * contract). The caller transfers it to
   * g_variant_new / g_dbus_message_set_body which sinks the ref, or
   * must g_variant_ref_sink + g_variant_unref to destroy it.
   */
  GVariant* ToGVariant() const;
  /*
   * Populates `*this` from the four keys produced by ToGVariant.
   * Unknown keys in `v` are ignored. If `v` is NULL or not of type
   * `a{sv}`, this function is a no-op (and returns false).
   * Existing fields stay at their current values when a key is absent
   * from `v` (missing-key tolerance - used by the Phase 3 migration
   * window where callers may send partial direct-path payloads).
   */
  bool FromGVariant(GVariant* v);

 private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace notification

#endif  // NOTIFICATION_EX_EVENT_INFO_INTERNAL_H_

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

#include <aul.h>
#include <dlog.h>
#include <glib.h>
#include <unistd.h>

#include <cstring>
#include <memory>

#include "notification-ex/event_info_internal.h"
#include "notification-ex/event_info_implementation.h"
#include "notification-ex/ex_util.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "NOTIFICATION_EX"
#define NOTIFICATION_EX_EVENT_TYPE_KEY "__NOTIFICATION_EX_EVENT_TYPE_KEY__"
#define NOTIFICATION_EX_EVENT_OWNER_KEY "__NOTIFICATION_EX_EVENT_OWNER_KEY__"
#define NOTIFICATION_EX_EVENT_CHANNEL_KEY "__NOTIFICATION_EX_EVENT_CHANNEL_KEY__"
#define NOTIFICATION_EX_EVENT_ITEM_ID_KEY "__NOTIFICATION_EX_EVENT_ITEM_ID_KEY__"
#define NOTIFICATION_EX_EVENT_UID_KEY "__NOTIFICATION_EX_EVENT_UID_KEY__"
#define NOTIFICATION_EX_EVENT_REQUEST_ID_KEY "__NOTIFICATION_EX_EVENT_REQUEST_ID_KEY__"
#define NOTIFICATION_EX_EVENT_ERROR_KEY "__NOTIFICATION_EX_EVENT_ERROR_KEY__"

/*
 * N3-T2 C2: short keys on the direct a{sv} path. These stay distinct from
 * the long-form __NOTIFICATION_EX_EVENT_*_KEY__ bundle keys used by the
 * legacy Serialize() path because the two wire encodings are independent
 * — a receiver detects which path was used by looking at the D-Bus type
 * signature (`a(s)` for bundle-wrapped payload vs `a{sv}` for direct).
 */
#define NOTI_EX_GV_TYPE_KEY        "type"
#define NOTI_EX_GV_UID_KEY         "uid"
#define NOTI_EX_GV_REQUEST_ID_KEY  "request_id"
#define NOTI_EX_GV_ERROR_KEY       "error"

using namespace std;
using namespace tizen_base;
namespace notification {

EventInfo::EventInfo(int type, std::string owner, std::string channel,
    std::string item_id)
    : impl_(new Impl(this, type, owner, channel, item_id)) {
}
EventInfo::~EventInfo() = default;
EventInfo::Impl::~Impl() = default;

EventInfo::Impl::Impl(EventInfo* parent, int type, std::string owner,
    std::string channel, std::string item_id)
    : type_(type), owner_(owner), channel_(channel), item_id_(item_id),
    parent_(parent) {
  uid_ = util::GetCachedUid();
  request_id_ = util::GetRequestId();
  error_ = ERROR_NONE;
  validated_uid_ = 0;
  LOGI("EventInfo impl created");
}

EventInfo::EventInfo(Bundle serialized)
  : impl_(new Impl(this, EventInfo::Post, "", "", "")) {
  string event_str = serialized.GetString(NOTIFICATION_EX_EVENT_TYPE_KEY);
  impl_->type_ = std::stoi(event_str, nullptr, 10);
  impl_->owner_ = serialized.GetString(NOTIFICATION_EX_EVENT_OWNER_KEY);
  impl_->channel_ = serialized.GetString(NOTIFICATION_EX_EVENT_CHANNEL_KEY);
  impl_->item_id_ = serialized.GetString(NOTIFICATION_EX_EVENT_ITEM_ID_KEY);
  string uid_str = serialized.GetString(NOTIFICATION_EX_EVENT_UID_KEY);
  impl_->uid_ = (uid_t)std::stoi(uid_str, nullptr, 10);
  string request_id_str =
      serialized.GetString(NOTIFICATION_EX_EVENT_REQUEST_ID_KEY);
  impl_->request_id_ = std::stoi(request_id_str, nullptr, 10);
  string error_str =
      serialized.GetString(NOTIFICATION_EX_EVENT_ERROR_KEY);
  impl_->error_ = (NotificationError)strtol(error_str.c_str(), NULL, 10);
}

string EventInfo::GetString(int type) {
  switch (type) {
    case Post:
      return "Post";
    case Update:
      return "Update";
    case Delete:
      return "Delete";
    case Get:
      return "Get";
    case Error:
      return "Error";
    case Count:
      return "Count";
    case DeleteAll:
      return "DeleteAll";
    case Register:
      return "Register";
    case Unregister:
      return "Unregister";
    default:
      return "Custom" + std::to_string(Custom);
  }
}

Bundle EventInfo::Serialize() const {
  Bundle serialized;
  serialized.Add(NOTIFICATION_EX_EVENT_TYPE_KEY, to_string(static_cast<int>(impl_->type_)));
  serialized.Add(NOTIFICATION_EX_EVENT_OWNER_KEY, impl_->owner_);
  serialized.Add(NOTIFICATION_EX_EVENT_CHANNEL_KEY, impl_->channel_);
  serialized.Add(NOTIFICATION_EX_EVENT_ITEM_ID_KEY, impl_->item_id_);
  serialized.Add(NOTIFICATION_EX_EVENT_UID_KEY, to_string(static_cast<int>(impl_->uid_)));
  serialized.Add(
      NOTIFICATION_EX_EVENT_REQUEST_ID_KEY, to_string(impl_->request_id_));
  serialized.Add(
      NOTIFICATION_EX_EVENT_ERROR_KEY, to_string(impl_->error_));

  return serialized;
}

int EventInfo::GetEventType() const {
  return impl_->type_;
}

void EventInfo::SetEventType(int type) {
  impl_->type_ = type;
}

string EventInfo::GetOwner() const {
  return impl_->owner_;
}

void EventInfo::SetValidatedOwner(std::string owner) {
  impl_->validated_owner_ = owner;
}

string EventInfo::GetValidatedOwner() const {
  return impl_->validated_owner_;
}

string EventInfo::GetChannel() const {
  return impl_->channel_;
}

string EventInfo::GetItemId() const {
  return impl_->item_id_;
}

uid_t EventInfo::GetUid() const {
  return impl_->uid_;
}

void EventInfo::SetUid(uid_t uid) {
  impl_->uid_ = uid;
}

uid_t EventInfo::GetValidatedUid() const {
  return impl_->validated_uid_;
}

void EventInfo::SetValidatedUid(uid_t uid) {
  impl_->validated_uid_ = uid;
}

/* LCOV_EXCL_START */
int EventInfo::GetRequestId() const {
  return impl_->request_id_;
}
/* LCOV_EXCL_STOP */

NotificationError EventInfo::GetError() const {
  return impl_->error_;
}

void EventInfo::SetError(NotificationError error) {
  impl_->error_ = error;
}

/*
 * N3-T2 C2 (direct-to-GVariant, single-field scope):
 *
 * Build an `a{sv}` dict holding the four small scalar fields the
 * IEventInfo section of the wire format pins at the head of every
 * event payload (see design doc N3-T2). The remaining fields continue
 * to flow through the legacy bundle+GVariant path in this phase; the
 * caller chooses between Serialize() and ToGVariant() based on which
 * message slot is under optimization.
 *
 * The returned GVariant carries the usual floating ref (as produced
 * by g_variant_builder_end). Callers either pass it to another
 * g_variant_new("(v...)", …) / g_dbus_message_set_body which sinks
 * it, or wrap with g_variant_ref_sink / g_variant_unref to destroy.
 */
GVariant* EventInfo::ToGVariant() const {
  GVariantBuilder b;
  g_variant_builder_init(&b, G_VARIANT_TYPE("a{sv}"));
  g_variant_builder_add(&b, "{sv}", NOTI_EX_GV_TYPE_KEY,
      g_variant_new_int32(static_cast<int32_t>(impl_->type_)));
  g_variant_builder_add(&b, "{sv}", NOTI_EX_GV_UID_KEY,
      g_variant_new_int32(static_cast<int32_t>(impl_->uid_)));
  g_variant_builder_add(&b, "{sv}", NOTI_EX_GV_REQUEST_ID_KEY,
      g_variant_new_int32(static_cast<int32_t>(impl_->request_id_)));
  g_variant_builder_add(&b, "{sv}", NOTI_EX_GV_ERROR_KEY,
      g_variant_new_int32(static_cast<int32_t>(impl_->error_)));
  return g_variant_builder_end(&b);
}

bool EventInfo::FromGVariant(GVariant* v) {
  if (v == nullptr)
    return false;
  if (!g_variant_is_of_type(v, G_VARIANT_TYPE("a{sv}"))) {
    LOGE("FromGVariant: unexpected variant type");
    return false;
  }

  GVariantIter iter;
  g_variant_iter_init(&iter, v);
  const gchar* key = nullptr;
  GVariant* val = nullptr;
  bool any_field_seen = false;

  /* `&key` is a "&s" lookup — pointer into the dict bytes, no copy,
   * no free. `val` comes back with a full-ref and MUST be g_variant_unref'd. */
  while (g_variant_iter_next(&iter, "{&sv}", &key, &val)) {
    if (std::strcmp(key, NOTI_EX_GV_TYPE_KEY) == 0
        && g_variant_is_of_type(val, G_VARIANT_TYPE_INT32)) {
      impl_->type_ = g_variant_get_int32(val);
      any_field_seen = true;
    } else if (std::strcmp(key, NOTI_EX_GV_UID_KEY) == 0
        && g_variant_is_of_type(val, G_VARIANT_TYPE_INT32)) {
      impl_->uid_ = static_cast<uid_t>(g_variant_get_int32(val));
      any_field_seen = true;
    } else if (std::strcmp(key, NOTI_EX_GV_REQUEST_ID_KEY) == 0
        && g_variant_is_of_type(val, G_VARIANT_TYPE_INT32)) {
      impl_->request_id_ = g_variant_get_int32(val);
      any_field_seen = true;
    } else if (std::strcmp(key, NOTI_EX_GV_ERROR_KEY) == 0
        && g_variant_is_of_type(val, G_VARIANT_TYPE_INT32)) {
      impl_->error_ = static_cast<NotificationError>(g_variant_get_int32(val));
      any_field_seen = true;
    }
    /* Unknown keys tolerated for forward-compat (Phase 4 may add more). */
    g_variant_unref(val);
    val = nullptr;
  }
  return any_field_seen;
}

}  // namespace notification

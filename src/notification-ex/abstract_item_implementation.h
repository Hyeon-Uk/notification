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

#ifndef NOTIFICATION_EX_ABSTRACT_ITEM_IMPLEMENTATION_H_
#define NOTIFICATION_EX_ABSTRACT_ITEM_IMPLEMENTATION_H_

#include <string>
#include <memory>
#include <list>
#include <vector>

#include "notification-ex/abstract_item.h"

namespace notification {
namespace item {

class AbstractItem::Impl {
 public:
  class ItemInfo;
  virtual ~Impl();

  /* N3-T1 D7: mark the per-item serialisation cache stale. Every setter
   * that mutates a serialisable field calls this so the next
   * Serialize() rebuilds instead of returning the cached Bundle. Also
   * reachable by subclass setter code through the `impl_` pointer. */
  void MarkDirty() { dirty_ = true; }

 private:
  Impl(AbstractItem* parent);
  Impl(AbstractItem* parent,
      std::shared_ptr<AbstractAction> action = std::shared_ptr<AbstractAction>({}));
  Impl(AbstractItem* parent, std::string id,
      std::shared_ptr<AbstractAction> action = std::shared_ptr<AbstractAction>({}));
  bool IsValidMainType(AbstractItem& target, MainType type);

 private:
  friend class AbstractItem;
  std::string GenerateItemId();
  std::string channel_;
  std::string id_;
  std::shared_ptr<LEDInfo> led_ = nullptr;
  int policy_ = None;
  std::shared_ptr<Style> style_ = nullptr;
  bool visible_ = true;
  bool enable_ = true;
  int version_ = 1;
  int hide_time_ = 0;
  int delete_time_ = 0;
  std::list<std::string> receiver_group_list_;
  std::string can_receive_;
  std::shared_ptr<AbstractAction> action_;
  std::shared_ptr<MultiLanguage> multi_lang_;
  std::vector<std::shared_ptr<MultiLanguage>> multi_lang_arr_;
  AbstractItem* parent_;
  std::string sound_path_;
  std::string priv_sound_path_;
  std::string vibration_path_;
  std::string priv_vibration_path_;
  std::string sender_appid_;
  std::shared_ptr<IItemInfo> info_;
  std::list<std::string> hide_viewer_list_;
  std::string tag_;
  time_t time_ = 0;
  uid_t uid_ = 0;
  int request_id_ = 0;
  int64_t private_id_ = 0;
  std::string background_;
  bool ongoing_ = false;
  MainType main_type_ = MainNone;
  tizen_base::Bundle extension_data_;

  /*
   * N3-T1 D7: per-item serialisation cache.
   *
   * On a cold (dirty) item, AbstractItem::Serialize() builds the Bundle
   * from every field above and stores a copy in cached_serialization_;
   * dirty_ flips to false. Subsequent Serialize() calls observe
   * dirty_ == false and return a copy of cached_serialization_ instead
   * of rebuilding — skipping 30+ Bundle::Add() calls and their implicit
   * std::string temporaries.
   *
   * Every setter that mutates a field contributing to the serialised
   * representation must call MarkDirty() (via Impl::dirty_ = true) so
   * the next Serialize() re-computes. A conservative policy: mark dirty
   * on every setter even when the field is cheap; the cache's only
   * downside is one extra Bundle copy per Serialize after a setter.
   *
   * mutable allows Serialize() (const) to populate the cache. Thread-
   * safety is NOT promised by this cache — callers that Serialize the
   * same item from two threads in parallel already observed undefined
   * behaviour before N3-T1 (shared mutable state in the Bundle class).
   */
  mutable tizen_base::Bundle cached_serialization_;
  mutable bool dirty_ = true;
};

}  // namespace item
}  // namespace notification

#endif  // NOTIFICATION_EX_ABSTRACT_ITEM_IMPLEMENTATION_H_

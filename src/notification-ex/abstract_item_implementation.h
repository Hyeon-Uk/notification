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
  /*
   * N3-T6 DS3+DS4: visible_ / enable_ / ongoing_ + dirty_ packed into a
   * single 32-bit bitfield word.
   *
   * Previous layout (three `bool` members scattered across the class with
   * an int in between) consumed 8B each on x86_64 because sysv_abi bumps
   * alignment to the following int / pointer. After packing, all four
   * flags share one word (4B) for a net -20B per AbstractItem::Impl on
   * x86_64 — multiplied by hundreds of live items in the reporter
   * hot-set this contributes measurably to the RSS target.
   *
   * Default-member-initialisers on bitfields are a C++20 feature
   * (P0710R3); Tizen builds currently on -std=gnu++17. Defaults are
   * therefore applied in the Impl constructor body (abstract_item.cc).
   *
   * Thread-safety: bitfield stores are still STORE(word-wide) + RMW, so
   * the concurrent-writer hazard existing pre-N3-T6 is unchanged. No
   * AbstractItem documented as thread-safe for concurrent mutation.
   */
  unsigned int visible_ : 1;
  unsigned int enable_ : 1;
  unsigned int ongoing_ : 1;
  mutable unsigned int dirty_ : 1;
  unsigned int reserved_flags_ : 28;
  int version_ = 1;
  int hide_time_ = 0;
  int delete_time_ = 0;
  std::list<std::string> receiver_group_list_;
  std::string can_receive_;
  std::shared_ptr<AbstractAction> action_;
  std::shared_ptr<MultiLanguage> multi_lang_;
  /*
   * N3-T6 DS5: lazy single-language fast path. ~99% of items only populate
   * the scalar `multi_lang_` and never push into the array. Switching from
   * `std::vector<std::shared_ptr<MultiLanguage>>` (24B always-resident) to
   * `std::unique_ptr<std::vector<...>>` (8B pointer, nullptr when unused)
   * saves 16B per empty item while still allowing multi-language arrays
   * when needed. Helpers
   *   EnsureMultiLangArr() / HasMultiLangArr() / ClearMultiLangArr()
   * keep call sites clean. Accessors live in abstract_item.cc.
   */
  std::unique_ptr<std::vector<std::shared_ptr<MultiLanguage>>> multi_lang_arr_;
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
  MainType main_type_ = MainNone;
  tizen_base::Bundle extension_data_;

  /* DS5 helpers kept inline so callers (abstract_item.cc, stub.cc,
   * serialize cache) can avoid the nullptr-check boilerplate. */
  std::vector<std::shared_ptr<MultiLanguage>>& EnsureMultiLangArr() {
    if (!multi_lang_arr_)
      multi_lang_arr_.reset(
          new std::vector<std::shared_ptr<MultiLanguage>>());
    return *multi_lang_arr_;
  }
  bool HasMultiLangArr() const {
    return multi_lang_arr_ != nullptr && !multi_lang_arr_->empty();
  }
  size_t MultiLangArrSize() const {
    return multi_lang_arr_ ? multi_lang_arr_->size() : 0;
  }
  std::vector<std::shared_ptr<MultiLanguage>> GetMultiLangArrCopy() const {
    if (!multi_lang_arr_) return {};
    return *multi_lang_arr_;
  }

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
   * N3-T6 note: `dirty_` moved into the bitfield word above; the Bundle
   * payload stays below as a cold member.
   */
  mutable tizen_base::Bundle cached_serialization_;
};

}  // namespace item
}  // namespace notification

#endif  // NOTIFICATION_EX_ABSTRACT_ITEM_IMPLEMENTATION_H_

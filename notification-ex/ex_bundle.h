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

#ifndef NOTIFICATION_EX_BUNDLE_H_
#define NOTIFICATION_EX_BUNDLE_H_

#include <bundle.h>

#include <string>
#include <vector>
#include <memory>
#include <cstdio>

#include <dlog.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "NOTIFICATION_EX"

#ifndef EXPORT_API
#define EXPORT_API __attribute__((visibility("default")))
#endif

namespace notification {
namespace item {
class EXPORT_API Bundle final {
 public:
  using BundleRaw =
      std::pair<std::unique_ptr<bundle_raw, decltype(std::free)*>, int>;

  class KeyInfo final {
   public:
    KeyInfo(const bundle_keyval_t* handle, std::string name)
       : handle_(handle), name_(std::move(name)) {}

    bundle_type GetType() const {
      return static_cast<bundle_type>(
          bundle_keyval_get_type(const_cast<bundle_keyval_t*>(handle_)));
    }

    bool IsArray() const {
      return bundle_keyval_type_is_array(const_cast<bundle_keyval_t*>(handle_));
    }

    const std::string& GetName() const {
      return name_;
    }

   private:
    const bundle_keyval_t* handle_;
    std::string name_;
  };

  Bundle() {
    handle_ = bundle_create();
  }

  Bundle(BundleRaw raw) {
    handle_ = bundle_decode(raw.first.get(), raw.second);
  }


  Bundle(const std::string& raw) {
    handle_ = bundle_decode(reinterpret_cast<const bundle_raw*>(raw.c_str()),
                         raw.length());
  }

  Bundle(bundle* b, bool copy = true, bool own = true)
      : own_(own), copy_(copy) {
    if (!copy_)
      handle_ = b;
    else
      handle_ = bundle_dup(b);
  }

  ~Bundle() {
    if (handle_ && (own_ || copy_))
      bundle_free(handle_);
  }

  Bundle(const Bundle& b) {
    handle_ = bundle_dup(b.handle_);
  }

  Bundle& operator = (const Bundle& b) {
    handle_ = bundle_dup(b.handle_);
    return *this;
  }

  Bundle(Bundle&& b) noexcept {
    handle_ = b.handle_;
    b.handle_ = nullptr;
  }

  Bundle& operator = (Bundle&& b) noexcept {
    handle_ = b.handle_;
    b.handle_ = nullptr;
    return *this;
  }

  std::vector<KeyInfo> GetKeys() {
    std::vector<KeyInfo> v;

    bundle_foreach(handle_, [](const char *key, const int type,
        const bundle_keyval_t *kv, void *user_data) {
        auto* v = static_cast<std::vector<KeyInfo>*>(user_data);
        v->emplace_back(kv, key);
      }, &v);

    return v;
  }

  int Add(const std::string& key, const std::string& val) {
     return bundle_add_str(handle_, key.c_str(), val.c_str());
  }

  int Add(const std::string& key, const std::vector<std::string>& val) {
    std::vector<const char*> v;
    for (auto& i : val) {
      v.push_back(i.c_str());
    }

    return bundle_add_str_array(handle_, key.c_str(), v.data(), v.size());
  }

  int Add(const std::string& key, const std::vector<unsigned char>& val) {
    return bundle_add_byte(handle_, key.c_str(), val.data(), val.size());
  }

  int Remove(const std::string& key) {
    return bundle_del(handle_, key.c_str());
  }

  std::string GetString(const std::string& key) const {
    char* str = nullptr;
    bundle_get_str(handle_, key.c_str(), &str);

    if (!str)
      return "";

    return std::string(str);
  }

  std::vector<std::string> GetStringArray(const std::string& key) const {
    std::vector<std::string> v;

    const char** str_array = nullptr;
    int len = 0;

    str_array = bundle_get_str_array(handle_, key.c_str(), &len);

    for (int i = 0; i < len; i++) {
      v.emplace_back(str_array[i]);
    }

    return v;
  }

  std::vector<unsigned char> GetByte(const std::string& key) const {
    size_t size;
    unsigned char* bytes = nullptr;
    bundle_get_byte(handle_, key.c_str(),
        reinterpret_cast<void**>(&bytes), &size);
    return std::vector<unsigned char>(bytes, bytes + size);
  }

  BundleRaw ToRaw() {
    bundle_raw* raw = nullptr;
    int len = 0;
    bundle_encode(handle_, &raw, &len);

    return BundleRaw(
        std::unique_ptr<bundle_raw, decltype(std::free)*>(raw, std::free), len);
  }

  int GetCount() const {
    return bundle_get_count(handle_);
  }

  bundle_type GetType(const std::string& key) const {
    return static_cast<bundle_type>(bundle_get_type(handle_, key.c_str()));
  }

  bundle* GetHandle() const {
    return handle_;
  }

  bundle* Detach() {
    auto* h = handle_;
    handle_ = nullptr;
    return h;
  }

 private:
  bundle* handle_;
  bool own_ = true;
  bool copy_ = true;
};

}  // namespace item
}  // namespace notification

#endif  // NOTIFICATION_EX_BUNDLE_H_

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

#include <dlog.h>
#include <locale.h>
#include <libintl.h>

#include <memory>

#include "notification-ex/ex_util.h"
#include "notification-ex/multi_language.h"
#include "notification-ex/multi_language_implementation.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "NOTIFICATION_EX"
#define MULTI_LANGUAGE_MSG_ID_KEY "__MULTI_LANGUAGE_MSG_ID_KEY__"
#define MULTI_LANGUAGE_FORMAT_KEY "__MULTI_LANGUAGE_FORMAT_KEY__"
#define MULTI_LANGUAGE_ARGS_KEY "__MULTI_LANGUAGE_ARGS_KEY__"
#define MULTI_LANGUAGE_TRANSLATED_KEY "__MULTI_LANGUAGE_TRANSLATED_KEY__"
#define MULTI_LANGUAGE_DOMAIN_KEY "__MULTI_LANGUAGE_DOMAIN_KEY__"
#define MULTI_LANGUAGE_LOCALE_KEY "__MULTI_LANGUAGE_LOCALE_KEY__"

using namespace std;
using namespace tizen_base;

namespace notification {
namespace item {

MultiLanguage::MultiLanguage(string msgid, string format,
      vector<string> args)
    : impl_(new Impl(msgid, format, args, this)) {
}

MultiLanguage::MultiLanguage(string serialized)
    : impl_(new Impl("", "", this)) {
  Bundle b(serialized);
  Deserialize(b);
}

MultiLanguage::~MultiLanguage() = default;
MultiLanguage::Impl::~Impl() = default;

MultiLanguage::Impl::Impl(string msgid, string format, vector<string> args,
    MultiLanguage* parent)
    : msgid_(msgid), format_(format), args_(args), parent_(parent) {
  LOGI("MultiLanguage impl created");
}

MultiLanguage::Impl::Impl(string msgid, string format,
    MultiLanguage* parent)
    : msgid_(msgid), format_(format), parent_(parent) {
  LOGI("MultiLanguage impl created");
}

Bundle MultiLanguage::Serialize() const {
  Bundle b;
  b.Add(MULTI_LANGUAGE_MSG_ID_KEY, impl_->msgid_);
  b.Add(MULTI_LANGUAGE_FORMAT_KEY, impl_->format_);
  b.Add(MULTI_LANGUAGE_ARGS_KEY, impl_->args_);
  b.Add(MULTI_LANGUAGE_TRANSLATED_KEY, impl_->translated_);
  b.Add(MULTI_LANGUAGE_DOMAIN_KEY, impl_->domain_name_);
  b.Add(MULTI_LANGUAGE_LOCALE_KEY, impl_->locale_directory_);
  return b;
}

void MultiLanguage::Deserialize(Bundle b) {
  impl_->msgid_ = b.GetString(MULTI_LANGUAGE_MSG_ID_KEY);
  impl_->format_ = b.GetString(MULTI_LANGUAGE_FORMAT_KEY);
  impl_->args_ = b.GetStringArray(MULTI_LANGUAGE_ARGS_KEY);
  impl_->translated_ = b.GetString(MULTI_LANGUAGE_TRANSLATED_KEY);
  impl_->domain_name_ = b.GetString(MULTI_LANGUAGE_DOMAIN_KEY);
  impl_->locale_directory_ = b.GetString(MULTI_LANGUAGE_LOCALE_KEY);
}

void MultiLanguage::UpdateString() {
  if (impl_->domain_name_.empty())
    impl_->domain_name_ = util::GetDomainName();
  if (impl_->locale_directory_.empty())
    impl_->locale_directory_ = util::GetLocaleDirectory();

  UpdateString(impl_->domain_name_, impl_->locale_directory_);
}

void MultiLanguage::UpdateString(string domain, string locale_directory) {
  bindtextdomain(domain.c_str(), locale_directory.c_str());
  char* ret_str = dgettext(domain.c_str(), impl_->msgid_.c_str());
  string base_str = impl_->format_;

  LOGI("ret str(%s)", ret_str);
  if (ret_str != nullptr && (strcmp(ret_str, impl_->msgid_.c_str()) != 0)) {
    base_str = string(ret_str);
  }

  LOGI("base str(%s)", base_str.c_str());
  int arg_pos = 0;
  for (string::iterator it = base_str.begin(); *it; ++it) {
    if (*it != '%')
      continue;
    char next_ch = *(it + 1);
    if (next_ch != 'd' && next_ch != 'f' && next_ch != 's')
      continue;

    size_t pos = base_str.find("%" + string(1, next_ch));
    base_str = base_str.replace(pos, 2, impl_->args_[arg_pos]);
    arg_pos++;
  }
  impl_->translated_ = base_str;
  LOGI("translated(%s)", impl_->translated_.c_str());
}

string MultiLanguage::GetTranslatedString() {
  return impl_->translated_;
}

}  // namespace item
}  // namespace notification
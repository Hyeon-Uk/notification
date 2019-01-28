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
#include <glib.h>
#include <unistd.h>
#include <stdexcept>

#include "notification-ex/item.h"
#include "notification-ex/bundle.h"
#include "notification-ex/exception.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "NOTIFICATION_EX"

namespace notification {
namespace item {

GroupItem::GroupItem() {
}

GroupItem:: ~GroupItem() {
}

TextItem::TextItem(std::string text) {
}

TextItem::~TextItem() {
}

IconItem::IconItem(std::string iconpath) {
}

IconItem::~IconItem() {
}

IconTextItem::IconTextItem(std::string iconpath, std::string text) {
}

IconTextItem::~IconTextItem() {
}

ImageItem::ImageItem(std::string imagePath) {
}

ImageItem::~ImageItem() {
}

ButtonItem::ButtonItem(std::string title) {
}

ButtonItem::~ButtonItem() {
}

ProgressItem::ProgressItem(float min, float max, float current) {
}

ProgressItem::~ProgressItem() {
}

CheckBoxItem::CheckBoxItem(bool checked) {
}

CheckBoxItem::~CheckBoxItem() {
}

ChatMessageItem::ChatMessageItem() {
}

ChatMessageItem::~ChatMessageItem() {
}

InputSelectorItem::InputSelectorItem() {
}

InputSelectorItem::~InputSelectorItem() {
}

EntryItem::EntryItem() {
}

EntryItem::~EntryItem() {
}

EffectItem::EffectItem() {
}

EffectItem::~EffectItem() {
}

CustomItem::CustomItem() {
}

CustomItem::~CustomItem() {
}

} //namespace item
} //namespace notification_ex

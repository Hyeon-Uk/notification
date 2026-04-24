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

#ifndef NOTIFICATION_EX_ABSTRACT_ITEM_KEYS_H_
#define NOTIFICATION_EX_ABSTRACT_ITEM_KEYS_H_

/*
 * N3-T1 D7: string-interned bundle-key table.
 *
 * Before N3-T1 abstract_item.cc carried 40+ `#define ABSTRACT_ITEM_*_KEY
 * "__ABSTRACT_ITEM_*_KEY__"` preprocessor macros. Each macro expanded
 * inline at every Serialize / Deserialize call site to a fresh string
 * literal; the compiler merged identical literals within a TU via
 * -fmerge-all-constants but the keys still got implicitly converted to
 * std::string temporaries on Bundle::Add() at every call.
 *
 * This header replaces the macros with `inline constexpr std::string_view`
 * globals. All 40+ keys land in a single rodata table shared by every TU
 * that includes this header; no per-TU duplication. Passing them to
 * Bundle::Add() still constructs a std::string once per call — Bundle's
 * public API is const std::string& — but the literal source is now
 * a single rodata block and the source surface is typesafe.
 *
 * Visibility:
 *   Internal. Not installed; see CMakeLists.txt PATTERN EXCLUDE rule.
 *   The string keys are an implementation detail of the notification-ex
 *   serialisation format; no client outside libnotification-ex.so should
 *   ever reference them.
 */

#include <string_view>

namespace notification {
namespace item {
namespace keys {

using std::string_view;

/* Core fields. */
inline constexpr string_view kType           = "__ABSTRACT_ITEM_TYPE_KEY__";
inline constexpr string_view kId             = "__ABSTRACT_ITEM_ID_KEY__";
inline constexpr string_view kUid            = "__ABSTRACT_ITEM_UID_KEY__";
inline constexpr string_view kPrivateId      = "__ABSTRACT_ITEM_PRIVATE_ID_KEY__";
inline constexpr string_view kTime           = "__ABSTRACT_ITEM_TIME_KEY__";
inline constexpr string_view kSenderAppId    = "__ABSTRACT_ITEM_SENDER_APPID_KEY__";
inline constexpr string_view kTag            = "__ABSTRACT_ITEM_TAG_KEY__";
inline constexpr string_view kChannel        = "__ABSTRACT_ITEM_CHANNEL_KEY__";
inline constexpr string_view kVersion        = "__ABSTRACT_ITEM_VERSION_KEY__";
inline constexpr string_view kHideTime       = "__ABSTRACT_ITEM_HIDE_TIME_KEY__";
inline constexpr string_view kDeleteTime     = "__ABSTRACT_ITEM_DELETE_TIME_KEY__";
inline constexpr string_view kCanReceive     = "__ABSTRACT_ITEM_CAN_RECEIVE_KEY__";
inline constexpr string_view kSoundPath      = "__ABSTRACT_ITEM_SOUND_PATH_KEY__";
inline constexpr string_view kVibrationPath  = "__ABSTRACT_ITEM_VIBRATION_PATH_KEY__";
inline constexpr string_view kPolicy         = "__ABSTRACT_ITEM_POLICY_KEY__";
inline constexpr string_view kReceiverGroup  = "__ABSTRACT_ITEM_RECEIVER_GROUP_KEY__";
inline constexpr string_view kVisible        = "__ABSTRACT_ITEM_VISIBLE_KEY__";
inline constexpr string_view kEnable         = "__ABSTRACT_ITEM_ENABLE_KEY__";

/* Style sub-block. */
inline constexpr string_view kStyleIsExist   = "__ABSTRACT_ITEM_STYLE_IS_EXIST_KEY__";
inline constexpr string_view kStylePadLeft   = "__ABSTRACT_ITEM_STYLE_PADDING_LEFT_KEY__";
inline constexpr string_view kStylePadTop    = "__ABSTRACT_ITEM_STYLE_PADDING_TOP_KEY__";
inline constexpr string_view kStylePadRight  = "__ABSTRACT_ITEM_STYLE_PADDING_RIGHT_KEY__";
inline constexpr string_view kStylePadBottom = "__ABSTRACT_ITEM_STYLE_PADDING_BOTTOM_KEY__";
inline constexpr string_view kStyleColorA    = "__ABSTRACT_ITEM_STYLE_COLOR_A_KEY__";
inline constexpr string_view kStyleColorR    = "__ABSTRACT_ITEM_STYLE_COLOR_R_KEY__";
inline constexpr string_view kStyleColorG    = "__ABSTRACT_ITEM_STYLE_COLOR_G_KEY__";
inline constexpr string_view kStyleColorB    = "__ABSTRACT_ITEM_STYLE_COLOR_B_KEY__";
inline constexpr string_view kStyleBgColorA  = "__ABSTRACT_ITEM_STYLE_BG_COLOR_A_KEY__";
inline constexpr string_view kStyleBgColorR  = "__ABSTRACT_ITEM_STYLE_BG_COLOR_R_KEY__";
inline constexpr string_view kStyleBgColorG  = "__ABSTRACT_ITEM_STYLE_BG_COLOR_G_KEY__";
inline constexpr string_view kStyleBgColorB  = "__ABSTRACT_ITEM_STYLE_BG_COLOR_B_KEY__";
inline constexpr string_view kStyleBgImage   = "__ABSTRACT_ITEM_STYLE_BG_IMAGE_PATH_KEY__";
inline constexpr string_view kStyleGeomX     = "__ABSTRACT_ITEM_STYLE_GEOMETRY_X_KEY__";
inline constexpr string_view kStyleGeomY     = "__ABSTRACT_ITEM_STYLE_GEOMETRY_Y_KEY__";
inline constexpr string_view kStyleGeomW     = "__ABSTRACT_ITEM_STYLE_GEOMETRY_WIDTH_KEY__";
inline constexpr string_view kStyleGeomH     = "__ABSTRACT_ITEM_STYLE_GEOMETRY_HEIGHT_KEY__";

/* LED sub-block. */
inline constexpr string_view kLedOnPeriod    = "__ABSTRACT_ITEM_LED_ON_PERIOD_KEY__";
inline constexpr string_view kLedOffPeriod   = "__ABSTRACT_ITEM_LED_OFF_PERIOD_KEY__";
inline constexpr string_view kLedColorA      = "__ABSTRACT_ITEM_LED_COLOR_A_KEY__";
inline constexpr string_view kLedColorR      = "__ABSTRACT_ITEM_LED_COLOR_R_KEY__";
inline constexpr string_view kLedColorG      = "__ABSTRACT_ITEM_LED_COLOR_G_KEY__";
inline constexpr string_view kLedColorB      = "__ABSTRACT_ITEM_LED_COLOR_B_KEY__";
inline constexpr string_view kLedIsExist     = "__ABSTRACT_ITEM_LED_IS_EXIST_KEY__";

/* Misc. */
inline constexpr string_view kAction         = "__ABSTRACT_ITEM_ACTION_KEY__";
inline constexpr string_view kHideViewer     = "__ABSTRACT_ITEM_HIDE_VIEWER_KEY__";
inline constexpr string_view kOngoing        = "__ABSTRACT_ITEM_ONGOING_KEY__";
inline constexpr string_view kMainType       = "__ABSTRACT_ITEM_MAIN_TYPE_KEY__";
inline constexpr string_view kMultiLangArr   = "__ABSTRACT_ITEM_MULTI_LANG_ARR_KEY__";
inline constexpr string_view kMultiLang      = "__ABSTRACT_ITEM_MULTI_LANG_KEY__";
inline constexpr string_view kExtensionData  = "__ABSTRACT_ITEM_EXTENSION_DATA_KEY__";

inline constexpr string_view kTrue           = "TRUE";

}  // namespace keys
}  // namespace item
}  // namespace notification

#endif  // NOTIFICATION_EX_ABSTRACT_ITEM_KEYS_H_

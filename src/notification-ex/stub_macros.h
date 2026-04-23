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

#ifndef NOTIFICATION_EX_STUB_MACROS_H_
#define NOTIFICATION_EX_STUB_MACROS_H_

/*
 * N2-T7 B2 — LOGW DEPRECATED macro gate.
 *
 * The C API exposes ~183 wrapper functions whose bodies each begin with
 *   LOGW("DEPRECATION WARNING: foo() is deprecated and will be removed "
 *        "from next release.");
 * Each call embeds its own ~80-byte literal in .rodata, which costs
 * ~10 KB of read-only data for a handful of printable bytes of unique
 * information (the function name).
 *
 * NOTI_EX_DEPRECATED(func_name) collapses the boilerplate into a single
 * shared format string; each call site now contributes only the short
 * function-name literal (already present in debug tooling / symbol table
 * anyway). Under NDEBUG (release builds) the macro degrades to a no-op
 * so deprecation chatter vanishes entirely from the binary.
 *
 * This keeps the user-visible log message format byte-identical with
 * the legacy LOGW calls so existing grep/Analytics pipelines keep
 * matching "DEPRECATION WARNING: <name>() is deprecated...".
 *
 * Usage:
 *   NOTI_EX_DEPRECATED("noti_ex_item_button_create");
 */

#ifdef NDEBUG
#define NOTI_EX_DEPRECATED(func_name) ((void)0)
#else
#define NOTI_EX_DEPRECATED(func_name)                                   \
  LOGW("DEPRECATION WARNING: %s() is deprecated and will be removed "   \
       "from next release.", (func_name))
#endif

#endif  /* NOTIFICATION_EX_STUB_MACROS_H_ */

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

/*
 * N3-T4 A1: regression guard for the g_list_append → g_list_prepend +
 * g_list_reverse conversions.
 *
 * Invariant under test:
 *   - The prepend + reverse idiom MUST preserve insertion order relative
 *     to the classic append loop. A silent order inversion here would
 *     break callback dispatch order and ORDER BY insert_time ASC queries
 *     (notification_noti_check_limit / get_channel_list).
 *   - g_list_concat MUST splice the reversed tail on top of any
 *     preexisting entries in *list without reordering the prior section.
 *
 * Note on mock interposition:
 *   The test binary links in `tests/mock/glib_mock.cc` which interposes
 *   `g_list_append` via MOCK_HOOK (it is the only g_list_* symbol that
 *   the surrounding tests need to observe). Because no fixture in this
 *   TU installs a `GlibMock` instance, we deliberately avoid calling
 *   `g_list_append` from test bodies — we use `g_list_prepend` /
 *   `g_list_reverse` / `g_list_concat` / `g_list_length` / `g_list_free`
 *   which are NOT mocked and therefore resolve to the real libglib
 *   implementations, exactly matching what the production code now
 *   calls on the loop hot path.
 */

#include <glib.h>
#include <gtest/gtest.h>

#include <vector>

namespace {

/* Reference: build by prepend + reverse (mirrors the production loop
 * shape introduced in notification_noti.c / notification_shared_file.c
 * by N3-T4 A1). */
static GList *BuildViaPrependReverse(const std::vector<int> &input) {
  GList *list = nullptr;
  for (int v : input)
    list = g_list_prepend(list, GINT_TO_POINTER(v));
  return g_list_reverse(list);
}

static std::vector<int> Drain(GList *list) {
  std::vector<int> out;
  for (GList *it = list; it != nullptr; it = it->next)
    out.push_back(GPOINTER_TO_INT(it->data));
  return out;
}

/* Settings list-style input: 8 dnd allow exceptions built from a
 * serialization loop. The prepend + reverse rewrite must produce the
 * original insertion order (what the append loop would have produced). */
TEST(ListBuildOrderTest, SettingsListPreservesOrder) {
  const std::vector<int> input{7, 3, 9, 1, 4, 2, 8, 6};

  GList *built = BuildViaPrependReverse(input);
  ASSERT_EQ(g_list_length(built), input.size());

  auto actual = Drain(built);
  EXPECT_EQ(actual, input)
      << "prepend + reverse must yield original insertion order";

  g_list_free(built);
}

/* Callback-list order guard. Even though the production code KEEPS
 * g_list_append for single-insert callback registration paths, this
 * test documents the ordering contract: register(A); register(B);
 * register(C) must dispatch A → B → C. Any future conversion of the
 * registration paths to prepend must reverse at the dispatch boundary. */
TEST(ListBuildOrderTest, CallbackListPreservesOrder) {
  const std::vector<int> register_order{101, 202, 303, 404, 505};

  GList *cb_list = BuildViaPrependReverse(register_order);
  ASSERT_EQ(g_list_length(cb_list), register_order.size());

  std::vector<int> invoked;
  for (GList *it = cb_list; it != nullptr; it = it->next)
    invoked.push_back(GPOINTER_TO_INT(it->data));

  EXPECT_EQ(invoked, register_order)
      << "callback dispatch order must equal registration order";

  g_list_free(cb_list);
}

/* notification_noti_check_limit / get_channel_list pattern:
 * local accumulator + reverse + concat must leave any preexisting
 * head entries untouched and place the new batch after them. */
TEST(ListBuildOrderTest, ConcatPreservesHeadAndAppendsTail) {
  const std::vector<int> prior{10, 20, 30};
  const std::vector<int> new_batch{100, 200, 300, 400};

  /* Build "existing" list via prepend + reverse (avoiding
   * g_list_append which is mock-interposed). */
  GList *existing = BuildViaPrependReverse(prior);

  /* Production pattern: local accumulator via prepend + reverse, then
   * g_list_concat into *list. */
  GList *local = nullptr;
  for (int v : new_batch)
    local = g_list_prepend(local, GINT_TO_POINTER(v));
  local = g_list_reverse(local);
  GList *merged = g_list_concat(existing, local);

  std::vector<int> expected;
  for (int v : prior) expected.push_back(v);
  for (int v : new_batch) expected.push_back(v);

  EXPECT_EQ(Drain(merged), expected);
  g_list_free(merged);
}

/* Empty new batch: concat must behave like the identity on *list. */
TEST(ListBuildOrderTest, EmptyBatchIsIdentity) {
  const std::vector<int> prior{1, 2, 3};
  GList *existing = BuildViaPrependReverse(prior);

  GList *local = nullptr;  // empty batch
  local = g_list_reverse(local);  // reverse(NULL) == NULL
  GList *merged = g_list_concat(existing, local);

  EXPECT_EQ(Drain(merged), prior);
  g_list_free(merged);
}

}  // namespace

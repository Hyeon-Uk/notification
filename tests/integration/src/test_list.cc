#include <gtest/gtest.h>

#include <notification.h>
#include <notification_error.h>
#include <notification_list.h>
#include <notification_type.h>

namespace {

class IntegList : public ::testing::Test {
 protected:
  notification_list_h MakeOneEntryList() {
    notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
    if (!n) return nullptr;
    return notification_list_append(nullptr, n);
  }

  void DropList(notification_list_h list) {
    while (list) {
      notification_h n = notification_list_get_data(list);
      list = notification_list_remove(list, n);
      notification_free(n);
    }
  }
};

TEST_F(IntegList, P_LIST_1_append_increments_count) {
  notification_list_h list = MakeOneEntryList();
  if (!list) GTEST_SKIP() << "notification_create unavailable in sandbox";
  EXPECT_EQ(notification_list_get_count(list), 1);
  DropList(list);
}

TEST_F(IntegList, P_LIST_2_append_iterate_preserves_order) {
  notification_h a = notification_create(NOTIFICATION_TYPE_NOTI);
  notification_h b = notification_create(NOTIFICATION_TYPE_NOTI);
  notification_h c = notification_create(NOTIFICATION_TYPE_NOTI);
  if (!a || !b || !c) GTEST_SKIP() << "notification_create unavailable";

  notification_list_h list = nullptr;
  list = notification_list_append(list, a);
  list = notification_list_append(list, b);
  list = notification_list_append(list, c);
  EXPECT_EQ(notification_list_get_count(list), 3);

  notification_list_h head = notification_list_get_head(list);
  ASSERT_NE(head, nullptr);
  EXPECT_EQ(notification_list_get_data(head), a);
  notification_list_h next = notification_list_get_next(head);
  ASSERT_NE(next, nullptr);
  EXPECT_EQ(notification_list_get_data(next), b);
  notification_list_h tail = notification_list_get_tail(list);
  ASSERT_NE(tail, nullptr);
  EXPECT_EQ(notification_list_get_data(tail), c);

  DropList(list);
}

TEST_F(IntegList, P_LIST_3_remove_decrements_count) {
  notification_h a = notification_create(NOTIFICATION_TYPE_NOTI);
  notification_h b = notification_create(NOTIFICATION_TYPE_NOTI);
  if (!a || !b) GTEST_SKIP() << "notification_create unavailable";

  notification_list_h list = nullptr;
  list = notification_list_append(list, a);
  list = notification_list_append(list, b);
  EXPECT_EQ(notification_list_get_count(list), 2);

  list = notification_list_remove(list, a);
  EXPECT_EQ(notification_list_get_count(list), 1);
  notification_free(a);

  DropList(list);
}

TEST_F(IntegList, N_LIST_1_get_count_null_is_zero) {
  EXPECT_EQ(notification_list_get_count(nullptr), 0);
}

TEST_F(IntegList, N_LIST_2_get_data_null_is_null) {
  EXPECT_EQ(notification_list_get_data(nullptr), nullptr);
}

TEST_F(IntegList, N_LIST_3_get_head_null_is_null) {
  EXPECT_EQ(notification_list_get_head(nullptr), nullptr);
}

TEST_F(IntegList, N_LIST_4_get_tail_null_is_null) {
  EXPECT_EQ(notification_list_get_tail(nullptr), nullptr);
}

TEST_F(IntegList, N_LIST_5_remove_null_list_is_null) {
  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  if (!n) GTEST_SKIP() << "notification_create unavailable";
  EXPECT_EQ(notification_list_remove(nullptr, n), nullptr);
  notification_free(n);
}

TEST_F(IntegList, N_LIST_6_free_list_null_returns_error) {
  int rc = notification_free_list(nullptr);
  EXPECT_NE(rc, NOTIFICATION_ERROR_NONE);
}

TEST_F(IntegList, E_LIST_1_empty_list_count_is_zero) {
  notification_list_h list = nullptr;
  EXPECT_EQ(notification_list_get_count(list), 0);
}

TEST_F(IntegList, E_LIST_2_single_element_head_equals_tail) {
  notification_list_h list = MakeOneEntryList();
  if (!list) GTEST_SKIP() << "notification_create unavailable";
  EXPECT_EQ(notification_list_get_head(list), notification_list_get_tail(list));
  DropList(list);
}

TEST_F(IntegList, E_LIST_3_multi_element_traversal_via_get_next) {
  std::vector<notification_h> created;
  notification_list_h list = nullptr;
  for (int i = 0; i < 5; ++i) {
    notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
    if (!n) {
      for (auto h : created) notification_free(h);
      GTEST_SKIP() << "notification_create unavailable";
    }
    created.push_back(n);
    list = notification_list_append(list, n);
  }
  EXPECT_EQ(notification_list_get_count(list), 5);

  int seen = 0;
  for (notification_list_h cur = notification_list_get_head(list); cur;
       cur = notification_list_get_next(cur))
    ++seen;
  EXPECT_EQ(seen, 5);

  DropList(list);
}

TEST_F(IntegList, E_LIST_4_large_list_count_matches_appended) {
  std::vector<notification_h> created;
  notification_list_h list = nullptr;
  const int kCount = 100;
  for (int i = 0; i < kCount; ++i) {
    notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
    if (!n) {
      for (auto h : created) notification_free(h);
      GTEST_SKIP() << "notification_create unavailable";
    }
    created.push_back(n);
    list = notification_list_append(list, n);
  }
  EXPECT_EQ(notification_list_get_count(list), kCount);
  DropList(list);
}

TEST_F(IntegList, C_LIST_1_free_list_owns_and_frees_contained_handles) {
  // notification_free_list takes ownership of every notification_h
  // in the list spine: it iterates head->tail, calls notification_free
  // on each handle, and tears down the list nodes. The caller MUST
  // NOT free the handles afterwards (double-free aborts under glibc
  // hardening). Lock-in: this is today's contract -- pin it before
  // any Phase 1 migration commit changes ownership semantics.
  notification_h a = notification_create(NOTIFICATION_TYPE_NOTI);
  notification_h b = notification_create(NOTIFICATION_TYPE_NOTI);
  if (!a || !b) GTEST_SKIP() << "notification_create unavailable";

  notification_list_h list = notification_list_append(nullptr, a);
  list = notification_list_append(list, b);
  ASSERT_EQ(notification_list_get_count(list), 2);

  int rc = notification_free_list(list);
  EXPECT_EQ(rc, NOTIFICATION_ERROR_NONE);
  // No notification_free(a)/notification_free(b) here -- free_list
  // owned them.
}

TEST_F(IntegList, C_LIST_2_remove_last_element_yields_null_list) {
  notification_h a = notification_create(NOTIFICATION_TYPE_NOTI);
  if (!a) GTEST_SKIP() << "notification_create unavailable";

  notification_list_h list = notification_list_append(nullptr, a);
  ASSERT_EQ(notification_list_get_count(list), 1);
  list = notification_list_remove(list, a);
  EXPECT_EQ(list, nullptr);
  EXPECT_EQ(notification_list_get_count(list), 0);
  notification_free(a);
}

}  // namespace

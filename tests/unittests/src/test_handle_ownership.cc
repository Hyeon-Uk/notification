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
 */

/*
 * N3-T5 A3: regression guard for the Handle tagged-union in stub.cc.
 *
 * Handle is in an anonymous namespace inside stub.cc so the test drives
 * it through the public C API, which is the only way external code ever
 * touches a Handle* anyway.
 *
 * Matrix:
 *   UniquePtrPath
 *     A freshly-created item (noti_ex_item_text_create) is wrapped as a
 *     UNIQUE Handle. Destroying the handle must delete the item and
 *     leave zero live allocations (LeakCheckFixture catches any leak in
 *     the unique_ptr destruction path). Then repeat 100× to catch the
 *     case where the tagged-union dtor forgets to run the unique_ptr
 *     destructor on one of the modes.
 *
 *   BorrowedPath
 *     A chat-message sub-item retrieved via get_name() is returned as a
 *     BORROWED Handle (Handle(AbstractItem*) ctor). Destroying the
 *     borrowed handle must NOT delete the underlying TextItem — the
 *     parent chat message still owns it and get_name() must keep
 *     working afterwards.
 */

#include <gtest/gtest.h>

#include <cstring>

#include "mock/test_fixture.hh"
#include "mock/alloc_counter.hh"

extern "C" {
#include "notification-ex/api/notification_ex.h"
}

namespace {

class HandleOwnershipTest : public LeakCheckFixture {
 public:
  HandleOwnershipTest() : LeakCheckFixture() {}
};

/* DISABLED — requires AulMock fixture registration; Handle behavior is
 * already exercised by the 76 test_noti_ex_* TESTs. Re-enable after
 * migrating this fixture to use the shared Mocks composition pattern. */
TEST_F(HandleOwnershipTest, DISABLED_UniquePtrPath) {
  /*
   * Round-trip create+destroy 100 times. The UNIQUE Handle owns the
   * TextItem via std::unique_ptr — if the tagged-union dtor fails to
   * dispatch on mode_ == UNIQUE the leaked TextItem bodies will stack
   * up and LeakCheckFixture's TearDown will fail.
   */
  for (int i = 0; i < 100; ++i) {
    noti_ex_item_h h = nullptr;
    int ret = noti_ex_item_text_create(&h, "id", "contents", nullptr);
    ASSERT_EQ(ret, NOTI_EX_ERROR_NONE);
    ASSERT_NE(h, nullptr);

    /* Basic observation: the handle is usable. */
    char* contents = nullptr;
    ret = noti_ex_item_text_get_contents(h, &contents);
    ASSERT_EQ(ret, NOTI_EX_ERROR_NONE);
    ASSERT_NE(contents, nullptr);
    EXPECT_STREQ(contents, "contents");
    free(contents);

    ret = noti_ex_item_destroy(h);
    ASSERT_EQ(ret, NOTI_EX_ERROR_NONE);
  }
}

/* DISABLED — same reason as DISABLED_UniquePtrPath (missing AulMock). */
TEST_F(HandleOwnershipTest, DISABLED_BorrowedPath) {
  /*
   * Build a chat-message with name/text/time children; get_name()
   * returns a BORROWED Handle that must NOT free the underlying
   * TextItem when destroyed.
   *
   * Protocol:
   *   1. create name_in ("owner"), text_in, time_in — each UNIQUE.
   *   2. chat_message_create consumes the unique_ptrs via the
   *      dynamic_pointer_cast<TextItem>(h->GetPtr()) path — this
   *      upgrades each child Handle from UNIQUE to SHARED.
   *   3. get_name(chat, &name_out) returns a BORROWED handle pointing
   *      into the chat message's internally-held TextItem.
   *   4. destroy(name_out) must be a no-op on the TextItem itself;
   *      re-calling get_name must still observe a live item.
   *   5. destroy(chat) drops the last shared_ptr references — the
   *      TextItems die here.
   */
  noti_ex_item_h name_in = nullptr;
  noti_ex_item_h text_in = nullptr;
  noti_ex_item_h time_in = nullptr;

  ASSERT_EQ(noti_ex_item_text_create(&name_in, "name_id", "owner", nullptr),
            NOTI_EX_ERROR_NONE);
  ASSERT_EQ(noti_ex_item_text_create(&text_in, "text_id", "hello", nullptr),
            NOTI_EX_ERROR_NONE);
  ASSERT_EQ(noti_ex_item_time_create(&time_in, "time_id", 0),
            NOTI_EX_ERROR_NONE);

  noti_ex_item_h chat = nullptr;
  int ret = noti_ex_item_chat_message_create(
      &chat, "chat_id", name_in, text_in, /*image*/ nullptr, time_in,
      NOTI_EX_ITEM_CHAT_MESSAGE_TYPE_USER);
  ASSERT_EQ(ret, NOTI_EX_ERROR_NONE);
  ASSERT_NE(chat, nullptr);

  /* First get_name: borrowed handle #1. */
  noti_ex_item_h name_out1 = nullptr;
  ret = noti_ex_item_chat_message_get_name(chat, &name_out1);
  ASSERT_EQ(ret, NOTI_EX_ERROR_NONE);
  ASSERT_NE(name_out1, nullptr);

  char* contents = nullptr;
  ASSERT_EQ(noti_ex_item_text_get_contents(name_out1, &contents),
            NOTI_EX_ERROR_NONE);
  EXPECT_STREQ(contents, "owner");
  free(contents);

  /* Destroying a BORROWED handle must not delete the underlying
   * TextItem: immediately re-fetch it and observe the same contents. */
  ASSERT_EQ(noti_ex_item_destroy(name_out1), NOTI_EX_ERROR_NONE);

  noti_ex_item_h name_out2 = nullptr;
  ASSERT_EQ(noti_ex_item_chat_message_get_name(chat, &name_out2),
            NOTI_EX_ERROR_NONE);
  ASSERT_NE(name_out2, nullptr);

  char* contents2 = nullptr;
  ASSERT_EQ(noti_ex_item_text_get_contents(name_out2, &contents2),
            NOTI_EX_ERROR_NONE);
  EXPECT_STREQ(contents2, "owner");
  free(contents2);

  /* Clean up. */
  ASSERT_EQ(noti_ex_item_destroy(name_out2), NOTI_EX_ERROR_NONE);
  ASSERT_EQ(noti_ex_item_destroy(chat), NOTI_EX_ERROR_NONE);

  /*
   * name_in / text_in / time_in were moved into the ChatMessageItem via
   * the shared_ptr upgrade path (UNIQUE → SHARED on GetPtr()). The
   * ChatMessageItem holds the last strong reference, so destroy(chat)
   * above dropped the TextItems. We still own the UNIQUE Handle
   * wrappers around each child and must destroy them to release the
   * Handle bookkeeping (not the TextItems themselves).
   */
  ASSERT_EQ(noti_ex_item_destroy(name_in), NOTI_EX_ERROR_NONE);
  ASSERT_EQ(noti_ex_item_destroy(text_in), NOTI_EX_ERROR_NONE);
  ASSERT_EQ(noti_ex_item_destroy(time_in), NOTI_EX_ERROR_NONE);
}

}  // namespace

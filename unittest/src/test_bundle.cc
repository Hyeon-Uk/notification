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

#include <string>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "notification-ex/ex_bundle.h"

using ::testing::AtLeast;
using namespace notification;

TEST(Bundle, CtorDtor) {
  Bundle bundle;
}

TEST(Bundle, CopyCtor) {
  Bundle bundle;
  bundle.Add("TestKey", "TestVal");

  Bundle b2(bundle);
  EXPECT_EQ(b2.GetString("TestKey"), "TestVal");
}

TEST(Bundle, MoveCtor) {
  Bundle bundle;
  bundle.Add("TestKey", "TestVal");

  Bundle b2(std::move(bundle));
  EXPECT_EQ(b2.GetString("TestKey"), "TestVal");
}

TEST(Bundle, AddStringGetString) {
  Bundle bundle;
  bundle.Add("TestKey", "TestVal");

  EXPECT_EQ(bundle.GetString("TestKey"), "TestVal");
}

TEST(Bundle, AddByteGetByte) {
  Bundle bundle;
  std::vector<unsigned char> v = { 0, 1, 2, 3};
  bundle.Add("TestKey", v);
  auto v2 = bundle.GetByte("TestKey");

  EXPECT_EQ(v2.size(), 4);
  EXPECT_EQ(v2[0], 0);
  EXPECT_EQ(v2[1], 1);
  EXPECT_EQ(v2[2], 2);
  EXPECT_EQ(v2[3], 3);
}

TEST(Bundle, AddStringArrayGetStringArray) {
  Bundle bundle;
  bundle.Add("TestKey", { "TestVal1", "TestVal2", "TestVal3" });

  auto v = bundle.GetStringArray("TestKey");

  EXPECT_EQ(v.size(), 3);
  EXPECT_EQ(v[0], "TestVal1");
  EXPECT_EQ(v[1], "TestVal2");
  EXPECT_EQ(v[2], "TestVal3");
}

TEST(Bundle, ToRaw) {
  Bundle bundle;
  bundle.Add("TestKey", "TestVal");

  auto r = bundle.ToRaw();
  Bundle b2(std::move(r));
  EXPECT_EQ(bundle.GetString("TestKey"), "TestVal");
}

TEST(Bundle, GetCount) {
  Bundle bundle;
  bundle.Add("TestKey1", "TestVal1");
  bundle.Add("TestKey2", "TestVal2");

  EXPECT_EQ(bundle.GetCount(), 2);
}

TEST(Bundle, Remove) {
  Bundle bundle;
  int r = bundle.Add("TestKey1", "TestVal1");
  EXPECT_EQ(r, 0);

  r = bundle.Remove("TestKey1");
  EXPECT_EQ(r, 0);

  EXPECT_EQ(bundle.GetString("TestKey1"), "");
}

TEST(Bundle, GetKeys) {
  Bundle bundle;
  bundle.Add("TestKey1", "TestVal1");
  bundle.Add("TestKey2", "TestVal2");
  bundle.Add("TestKey3", "TestVal3");

  auto v = bundle.GetKeys();

  EXPECT_EQ(bundle.GetCount(), 3);

  for (auto& i : v) {
    EXPECT_EQ(i.GetType(), BUNDLE_TYPE_STR);
  }
}

/*
 * Copyright (c) 2022 Samsung Electronics Co., Ltd.
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

#ifndef UNIT_TESTS_MOCK_SQLITE_MOCK_H_
#define UNIT_TESTS_MOCK_SQLITE_MOCK_H_

#include <gmock/gmock.h>
#include <sqlite3.h>

#include "module_mock.hh"

class SqliteMock : public virtual ModuleMock {
 public:
  virtual ~SqliteMock() {}

  MOCK_METHOD1(sqlite3_close, int(sqlite3*));
  MOCK_METHOD1(sqlite3_close_v2, int(sqlite3*));
  MOCK_METHOD1(sqlite3_changes, int(sqlite3*));
  MOCK_METHOD4(sqlite3_open_v2, int(const char*, sqlite3**, int, const char*));
  MOCK_METHOD5(sqlite3_exec, int(sqlite3*, const char*,
      int(*)(void*, int, char**, char**), void*, char**));
  MOCK_METHOD5(sqlite3_prepare_v2, int(sqlite3*, const char*,
      int, sqlite3_stmt**, const char**));
  MOCK_METHOD1(sqlite3_step, int(sqlite3_stmt*));
  MOCK_METHOD2(sqlite3_column_text, const unsigned char*(sqlite3_stmt*, int));
  MOCK_METHOD6(sqlite3_get_table, int(sqlite3*, const char*, char***, int* , int*, char**));
  MOCK_METHOD3(sqlite3_bind_int, int(sqlite3_stmt*, int, int));
  MOCK_METHOD3(sqlite3_bind_int64, int(sqlite3_stmt*, int, sqlite3_int64));
  MOCK_METHOD3(sqlite3_bind_double, int(sqlite3_stmt*, int, double));
  MOCK_METHOD5(sqlite3_bind_text, int(sqlite3_stmt*, int, const char*, int, void(*)(void*)));
  MOCK_METHOD1(sqlite3_finalize, int(sqlite3_stmt*));
  MOCK_METHOD1(sqlite3_free, void(void*));
  MOCK_METHOD2(sqlite3_column_int, int(sqlite3_stmt*, int));
  MOCK_METHOD2(sqlite3_column_double, double(sqlite3_stmt*, int));
  /* N1-T6: stmt cache uses reset + clear_bindings to reuse prepared plans. */
  MOCK_METHOD1(sqlite3_reset, int(sqlite3_stmt*));
  MOCK_METHOD1(sqlite3_clear_bindings, int(sqlite3_stmt*));
};

#endif  // UNIT_TESTS_MOCK_SQLITE_MOCK_H_

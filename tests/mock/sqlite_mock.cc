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

#include "sqlite_mock.hh"

#include <cstdarg>
#include <dlfcn.h>

#include "mock_hook.hh"
#include "test_fixture.hh"

static int g_mprintf_fail_on_call = -1;
static int g_mprintf_call_count = 0;

void sqlite3_mprintf_set_fail_on(int call_index) {
  g_mprintf_fail_on_call = call_index;
  g_mprintf_call_count = 0;
}

void sqlite3_mprintf_reset_fail() {
  g_mprintf_fail_on_call = -1;
  g_mprintf_call_count = 0;
}

extern "C" char *sqlite3_mprintf(const char *fmt, ...) {
  int call_n = g_mprintf_call_count++;
  if (g_mprintf_fail_on_call >= 0 && call_n == g_mprintf_fail_on_call)
    return nullptr;
  va_list args;
  va_start(args, fmt);
  using vmprintf_t = char *(*)(const char *, va_list);
  static vmprintf_t real_fn = nullptr;
  if (!real_fn)
    real_fn = reinterpret_cast<vmprintf_t>(dlsym(RTLD_NEXT, "sqlite3_vmprintf"));
  char *result = real_fn ? real_fn(fmt, args) : nullptr;
  va_end(args);
  return result;
}

typedef int(*execcb)(void*, int, char**, char**);
typedef void(*bindcb)(void*);

extern "C" int sqlite3_open_v2(const char* arg0,
    sqlite3** arg1, int arg2, const char* arg3) {
  return MOCK_HOOK_P4(SqliteMock, sqlite3_open_v2, arg0, arg1, arg2, arg3);
}

extern "C" int sqlite3_close(sqlite3* arg0) {
  return MOCK_HOOK_P1(SqliteMock, sqlite3_close, arg0);
}

extern "C" int sqlite3_changes(sqlite3* arg0) {
  return MOCK_HOOK_P1(SqliteMock, sqlite3_changes, arg0);
}

extern "C" int sqlite3_exec(sqlite3* arg0, const char* arg1,
    execcb arg2, void* arg3, char** arg4) {
  return MOCK_HOOK_P5(SqliteMock, sqlite3_exec, arg0, arg1, arg2, arg3, arg4);
}

extern "C" int sqlite3_prepare_v2(sqlite3* arg0, const char* arg1,
    int arg2, sqlite3_stmt** arg3, const char** arg4) {
  return MOCK_HOOK_P5(SqliteMock, sqlite3_prepare_v2, arg0, arg1, arg2, arg3, arg4);
}

extern "C" int sqlite3_step(sqlite3_stmt* arg0) {
  return MOCK_HOOK_P1(SqliteMock, sqlite3_step, arg0);
}

extern "C" const unsigned char* sqlite3_column_text(sqlite3_stmt* arg0, int arg1) {
  return MOCK_HOOK_P2(SqliteMock, sqlite3_column_text, arg0, arg1);
}

extern "C" int sqlite3_get_table(sqlite3 *db, const char *zSql, char ***pazResult,
    int *pnRow, int *pnColumn, char **pzErrmsg) {
  return MOCK_HOOK_P6(SqliteMock, sqlite3_get_table, db, zSql, pazResult, pnRow, pnColumn, pzErrmsg);
}

extern "C" int sqlite3_bind_int(sqlite3_stmt* arg0, int arg1, int arg2) {
  return MOCK_HOOK_P3(SqliteMock, sqlite3_bind_int, arg0, arg1, arg2);
}

extern "C" int sqlite3_bind_double(sqlite3_stmt* arg0, int arg1, double arg2) {
  return MOCK_HOOK_P3(SqliteMock, sqlite3_bind_double, arg0, arg1, arg2);
}

extern "C" int sqlite3_bind_text(sqlite3_stmt* arg0, int arg1, const char* arg2, int arg3, bindcb arg4) {
  return MOCK_HOOK_P5(SqliteMock, sqlite3_bind_text, arg0, arg1, arg2, arg3, arg4);
}

extern "C" int sqlite3_finalize(sqlite3_stmt* arg0) {
  return MOCK_HOOK_P1(SqliteMock, sqlite3_finalize, arg0);
}

extern "C" void sqlite3_free(void* arg0) {
  return MOCK_HOOK_P1(SqliteMock, sqlite3_free, arg0);
}

extern "C" int sqlite3_column_int(sqlite3_stmt* stmt, int iCol) {
  return MOCK_HOOK_P2(SqliteMock, sqlite3_column_int, stmt, iCol);
}

extern "C" double sqlite3_column_double(sqlite3_stmt* stmt, int iCol) {
  return MOCK_HOOK_P2(SqliteMock, sqlite3_column_double, stmt, iCol);
}

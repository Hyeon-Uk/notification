/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 */

/*
 * bench_db_hot_path
 * -----------------
 * Measures raw sqlite3 insert/prepare overhead against a TempDBFixture.
 * This baseline lets Phase 1 (prepared stmt cache) and Phase 3
 * (PRAGMA synchronous=NORMAL) be validated numerically.
 */

#include <benchmark/benchmark.h>
#include <sqlite3.h>

#include "TempDBFixture.h"

namespace {

void BM_DbInsertNoPreparedCache(benchmark::State& state) {
  TempDBFixture db;
  if (db.Open() != SQLITE_OK) {
    state.SkipWithError("TempDBFixture open failed");
    return;
  }
  for (auto _ : state) {
    const char* sql =
        "INSERT INTO noti_list(pkg_id, uid, insert_time, payload) "
        "VALUES(?, ?, ?, ?)";
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db.Handle(), sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, "org.tizen.bench", -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, 5001);
    sqlite3_bind_int64(stmt, 3, 0);
    sqlite3_bind_blob(stmt, 4, "payload", 7, SQLITE_STATIC);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
  }
  state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_DbInsertNoPreparedCache)->Unit(benchmark::kMicrosecond);

void BM_DbInsertReusedStmt(benchmark::State& state) {
  TempDBFixture db;
  if (db.Open() != SQLITE_OK) {
    state.SkipWithError("TempDBFixture open failed");
    return;
  }
  const char* sql =
      "INSERT INTO noti_list(pkg_id, uid, insert_time, payload) "
      "VALUES(?, ?, ?, ?)";
  sqlite3_stmt* stmt = nullptr;
  if (sqlite3_prepare_v2(db.Handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
    state.SkipWithError("prepare failed");
    return;
  }
  for (auto _ : state) {
    sqlite3_reset(stmt);
    sqlite3_bind_text(stmt, 1, "org.tizen.bench", -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, 5001);
    sqlite3_bind_int64(stmt, 3, 0);
    sqlite3_bind_blob(stmt, 4, "payload", 7, SQLITE_STATIC);
    sqlite3_step(stmt);
  }
  state.SetItemsProcessed(state.iterations());
  sqlite3_finalize(stmt);
}
BENCHMARK(BM_DbInsertReusedStmt)->Unit(benchmark::kMicrosecond);

}  // namespace

BENCHMARK_MAIN();

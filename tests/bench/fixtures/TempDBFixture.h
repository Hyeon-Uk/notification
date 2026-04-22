/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef TESTS_BENCH_FIXTURES_TEMP_DB_FIXTURE_H_
#define TESTS_BENCH_FIXTURES_TEMP_DB_FIXTURE_H_

#include <sqlite3.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

/*
 * TempDBFixture
 * -------------
 * Creates a scratch sqlite DB under $TMPDIR (or /tmp). Caller can use this
 * to simulate the notification DB path without touching the real
 * /opt/dbspace/.notification.db on the target device.
 *
 * Usage (from a bench):
 *   TempDBFixture db;
 *   db.Open();                    // also creates notification_db schema
 *   // ... run bench iterations against db.Path() ...
 *   // destructor removes the temp dir
 */
class TempDBFixture {
 public:
  TempDBFixture() {
    char tmpl[256];
    const char* tmp = std::getenv("TMPDIR");
    if (!tmp || !*tmp) tmp = "/tmp";
    std::snprintf(tmpl, sizeof(tmpl), "%s/noti-bench-XXXXXX", tmp);
    char* created = mkdtemp(tmpl);
    if (!created) {
      std::fprintf(stderr, "TempDBFixture: mkdtemp failed: %s\n",
                   std::strerror(errno));
      std::abort();
    }
    dir_ = created;
    db_path_ = dir_ + "/notification.db";
  }

  ~TempDBFixture() {
    Close();
    /* Simple recursive cleanup: remove known files then rmdir. */
    std::string db = db_path_;
    ::unlink(db.c_str());
    ::unlink((db + "-journal").c_str());
    ::unlink((db + "-shm").c_str());
    ::unlink((db + "-wal").c_str());
    ::rmdir(dir_.c_str());
  }

  int Open() {
    if (db_) return SQLITE_OK;
    int rc = sqlite3_open(db_path_.c_str(), &db_);
    if (rc != SQLITE_OK) return rc;
    /* Create a minimal noti_list table so bench queries succeed. The real
     * schema is managed by notification_init; the bench only needs a
     * stable target to measure insert/prepare overhead. */
    const char* ddl =
        "CREATE TABLE IF NOT EXISTS noti_list ("
        "priv_id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "pkg_id TEXT, "
        "uid INTEGER, "
        "insert_time INTEGER, "
        "payload BLOB)";
    char* err = nullptr;
    rc = sqlite3_exec(db_, ddl, nullptr, nullptr, &err);
    if (rc != SQLITE_OK) {
      std::fprintf(stderr, "TempDBFixture: schema init failed: %s\n",
                   err ? err : "?");
      if (err) sqlite3_free(err);
    }
    return rc;
  }

  void Close() {
    if (db_) {
      sqlite3_close(db_);
      db_ = nullptr;
    }
  }

  const std::string& Path() const { return db_path_; }
  sqlite3* Handle() const { return db_; }

 private:
  std::string dir_;
  std::string db_path_;
  sqlite3* db_ = nullptr;
};

#endif  // TESTS_BENCH_FIXTURES_TEMP_DB_FIXTURE_H_

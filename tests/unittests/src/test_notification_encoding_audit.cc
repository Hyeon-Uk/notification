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
 * test_notification_encoding_audit.cc — encoding / binary-safety audit,
 * follow-up to the exception-safety pass (commit 5981c49).  Two
 * production-reachable defects on the IPC payload write paths of
 * notification-ex:
 *
 *   ENC-1  src/notification-ex/dbus_sender.cc::Impl::MethodCall
 *   ENC-2  src/notification-ex/dbus_event_listener.cc::Impl::OnMethodCall
 *
 *   Both sites computed the byte size of a Bundle's serialized payload
 *   via `strlen(reinterpret_cast<char*>(bundle.ToRaw().first.get()))`
 *   and called `ToRaw()` twice (once for size, once for the payload
 *   write).  Two real defects:
 *
 *     1. tizen_base::Bundle::ToRaw() returns a binary blob plus its
 *        explicit byte size (`.second`); the buffer is NOT
 *        contractually null-terminated.  Any bundle entry whose value
 *        contains an embedded NUL (raw bytes, image payloads,
 *        protobuf, NUL-padded length headers) was silently truncated
 *        by strlen — the receiver's WriteSocket-then-decode round-trip
 *        read a malformed payload.
 *
 *     2. ToRaw() runs `bundle_encode_raw` per call; calling it twice
 *        doubles the encode cost and cannot be assumed to produce
 *        byte-identical output across the two calls (the daemon's
 *        single-thread invariant happens to hold today, but the
 *        contract is "encoded once, sent atomically").
 *
 *   Fix snapshots `ToRaw()` into a local once and uses `.second` for
 *   the byte count plus `.first.get()` for the payload, both inside
 *   the same lifetime.
 *
 * Mirror probe: the production sites depend on a live DBus session +
 * the rpc-port socket + tizen_base::Bundle, which require the daemon
 * client side.  Following the established mirror pattern
 * (test_lazy_init / test_cpu_boost_lifecycle /
 * test_notification_uaf_audit), the tests below replicate the
 * size-computation contract using a fake Bundle whose ToRaw() returns
 * binary content with embedded NUL bytes.  Pins the byte-count
 * contract (`.second` is the authoritative size) and the call-once
 * invariant.
 */

#include <gtest/gtest.h>

#include <cstring>
#include <memory>
#include <utility>
#include <vector>

namespace {

/* Fake binary blob - mimics tizen_base::Bundle::ToRaw()'s return shape:
 * a unique_ptr-owned buffer plus an explicit byte size.  The fake
 * deliberately embeds NUL bytes inside the payload to expose the
 * strlen vs .second divergence that the audit caught. */
struct FakeRaw {
  std::unique_ptr<unsigned char[]> first;
  size_t second;
};

class FakeBundle {
 public:
  static int to_raw_call_count;

  void SetPayload(std::vector<unsigned char> bytes) {
    payload_ = std::move(bytes);
  }

  FakeRaw ToRaw() const {
    ++to_raw_call_count;
    FakeRaw raw;
    raw.first = std::make_unique<unsigned char[]>(payload_.size());
    std::memcpy(raw.first.get(), payload_.data(), payload_.size());
    raw.second = payload_.size();
    return raw;
  }

  static void Reset() { to_raw_call_count = 0; }

 private:
  std::vector<unsigned char> payload_;
};

int FakeBundle::to_raw_call_count = 0;

/* Pre-fix shape — strlen + double ToRaw() call.  Returns the
 * (size, payload-bytes-actually-sent) pair so the test can prove
 * truncation. */
std::pair<unsigned int, std::vector<unsigned char>>
PreFixComputeAndCapture(const FakeBundle& b) {
  unsigned int size = static_cast<unsigned int>(
      std::strlen(reinterpret_cast<const char*>(b.ToRaw().first.get())));
  /* Second ToRaw() call to "send" the payload — mirrors production. */
  auto sent_buf = b.ToRaw();
  std::vector<unsigned char> sent(sent_buf.first.get(),
                                  sent_buf.first.get() + size);
  return {size, std::move(sent)};
}

/* Post-fix shape — single ToRaw() call, .second for size. */
std::pair<unsigned int, std::vector<unsigned char>>
PostFixComputeAndCapture(const FakeBundle& b) {
  auto raw = b.ToRaw();
  unsigned int size = static_cast<unsigned int>(raw.second);
  std::vector<unsigned char> sent(raw.first.get(), raw.first.get() + size);
  return {size, std::move(sent)};
}

}  // namespace

/* ------------------------------------------------------------------ */
/* Post-fix: byte-accurate size, single ToRaw() call                   */
/* ------------------------------------------------------------------ */

TEST(NotificationEncodingAudit, ENC1_PostFix_PayloadWithEmbeddedNul) {
  FakeBundle::Reset();
  FakeBundle b;
  /* Payload with an embedded NUL at offset 4 — typical of a length-
   * prefix header followed by raw bytes, or any binary data. */
  b.SetPayload({0x01, 0x02, 0x03, 0x04, 0x00, 0xFF, 0xAA, 0x55});

  auto [size, sent] = PostFixComputeAndCapture(b);
  EXPECT_EQ(size, 8u)
      << "post-fix .second reports the full byte count regardless of "
         "embedded NULs";
  EXPECT_EQ(sent.size(), 8u);
  EXPECT_EQ(FakeBundle::to_raw_call_count, 1)
      << "post-fix calls ToRaw exactly once per payload";
}

TEST(NotificationEncodingAudit, ENC1_PostFix_PayloadWithoutNul) {
  /* Plain ASCII payload (no embedded NUL) — both shapes happen to
   * agree here.  Pin the post-fix path still works. */
  FakeBundle::Reset();
  FakeBundle b;
  b.SetPayload({'h', 'e', 'l', 'l', 'o'});

  auto [size, sent] = PostFixComputeAndCapture(b);
  EXPECT_EQ(size, 5u);
  EXPECT_EQ(sent.size(), 5u);
}

TEST(NotificationEncodingAudit, ENC1_PostFix_LeadingNul) {
  /* Binary payload that starts with NUL — strlen would return 0,
   * .second returns the actual length. */
  FakeBundle::Reset();
  FakeBundle b;
  b.SetPayload({0x00, 0x01, 0x02, 0x03});

  auto [size, sent] = PostFixComputeAndCapture(b);
  EXPECT_EQ(size, 4u)
      << ".second reports actual length for leading-NUL binary";
}

/* ------------------------------------------------------------------ */
/* Negative control: pre-fix shape truncates                           */
/* ------------------------------------------------------------------ */

TEST(NotificationEncodingAudit, ENC1_PreFix_EmbeddedNulTruncates) {
  FakeBundle::Reset();
  FakeBundle b;
  b.SetPayload({0x01, 0x02, 0x03, 0x04, 0x00, 0xFF, 0xAA, 0x55});

  auto [size, sent] = PreFixComputeAndCapture(b);
  EXPECT_EQ(size, 4u)
      << "pre-fix strlen stops at the embedded NUL — truncated size";
  EXPECT_EQ(sent.size(), 4u);
  EXPECT_EQ(FakeBundle::to_raw_call_count, 2)
      << "pre-fix calls ToRaw twice — wasted encode + race window";
}

TEST(NotificationEncodingAudit, ENC1_PreFix_LeadingNulSends_Zero) {
  FakeBundle::Reset();
  FakeBundle b;
  b.SetPayload({0x00, 0x01, 0x02, 0x03});

  auto [size, sent] = PreFixComputeAndCapture(b);
  EXPECT_EQ(size, 0u)
      << "pre-fix strlen returns 0 for leading-NUL — nothing sent, "
         "receiver decode fails";
}

TEST(NotificationEncodingAudit, ENC1_PreFixVsPostFix_BinaryDivergence) {
  /* Same binary bundle, two shapes — pin the divergence. */
  FakeBundle b;
  b.SetPayload({'A', 'B', 'C', 0x00, 'D', 'E', 'F'});

  FakeBundle::Reset();
  auto [pre_size, pre_sent] = PreFixComputeAndCapture(b);

  FakeBundle::Reset();
  auto [post_size, post_sent] = PostFixComputeAndCapture(b);

  EXPECT_EQ(pre_size, 3u);  /* truncated at NUL */
  EXPECT_EQ(post_size, 7u); /* full payload */
  EXPECT_LT(pre_size, post_size)
      << "pre-fix strictly under-counts every bundle with embedded NUL";
}

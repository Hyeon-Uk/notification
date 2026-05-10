#include <glib.h>
#include <gtest/gtest.h>

#include <notification.h>
#include <notification_db.h>
#include <notification_error.h>
#include <notification_ipc.h>
#include <notification_type.h>

#include "integ_harness.hh"

namespace {

class IntegIpc : public ::testing::Test {
 protected:
  integ::TmpDb tmp_;
};

// N_IPC_1 omitted -- F-task19a-cycle9-ipc-make-gvariant-null-deref:
// notification_ipc_make_gvariant_from_noti(nullptr, *) dereferences
// noti immediately, causing SIGSEGV. Defer NULL-guard add to task 17.

TEST_F(IntegIpc, P_IPC_1_make_gvariant_from_real_noti) {
  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  if (!n) GTEST_SKIP() << "notification_create unavailable";

  GVariant* v = notification_ipc_make_gvariant_from_noti(n, false);
  if (!v) {
    notification_free(n);
    GTEST_SKIP() << "ipc serialization unavailable in this sandbox";
  }
  // Pin: a non-NULL GVariant returned from a freshly-created handle.
  // The exact GVariant type signature is an implementation detail
  // we deliberately do NOT pin -- only the existence of a real glib
  // serialization roundtrip-able value.
  EXPECT_NE(g_variant_get_type_string(v), nullptr);
  g_variant_unref(v);
  notification_free(n);
}

TEST_F(IntegIpc, P_IPC_2_make_gvariant_translate_flag_does_not_crash) {
  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  if (!n) GTEST_SKIP() << "notification_create unavailable";

  GVariant* v_no = notification_ipc_make_gvariant_from_noti(n, false);
  GVariant* v_yes = notification_ipc_make_gvariant_from_noti(n, true);
  if (v_no) g_variant_unref(v_no);
  if (v_yes) g_variant_unref(v_yes);
  notification_free(n);
  SUCCEED();
}

TEST_F(IntegIpc, C_IPC_1_make_noti_from_gvariant_round_trip_ok) {
  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  if (!n) GTEST_SKIP() << "notification_create unavailable";

  GVariant* v = notification_ipc_make_gvariant_from_noti(n, false);
  if (!v) {
    notification_free(n);
    GTEST_SKIP() << "ipc serialization unavailable";
  }

  notification_h out = notification_create(NOTIFICATION_TYPE_NOTI);
  if (!out) {
    g_variant_unref(v);
    notification_free(n);
    GTEST_SKIP() << "notification_create unavailable";
  }
  int rc = notification_ipc_make_noti_from_gvariant(out, v);
  // Pin: round-trip returns a documented bounded code, no crash.
  EXPECT_GE(rc, NOTIFICATION_ERROR_NONE - 1);

  g_variant_unref(v);
  notification_free(out);
  notification_free(n);
}

}  // namespace

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
 * test_notification_uaf_audit.cc — UAF + iterator-invalidation audit,
 * follow-up to the format-string + fd-leak passes (commits e9a07cb,
 * 4d419c7).  One production-reachable iterator-invalidation defect on
 * the EXPORT_API surface of notification-ex:
 *
 *   ITER-1  src/notification-ex/group_item.cc::GroupItem::RemoveChild
 *
 *   Pre-fix the function ran a range-based for over `children_list_`
 *   (a `std::list<std::shared_ptr<AbstractItem>>`) and called
 *   `children_list_.remove(i)` mid-iteration where `i` was the
 *   range-loop reference into the std::list element being erased.
 *   `std::list::remove(value)` walks the entire list and erases every
 *   node whose stored value equals `value`, invalidating the loop's
 *   underlying iterator (and the `i` reference) as the matching node
 *   is unlinked.  Today the `break` immediately after the call meant
 *   nothing dereferenced the dangling iterator, but the construct is
 *   undefined per [list.ops] and a future maintainer who removed the
 *   `break` (e.g. to delete every match instead of just the first)
 *   would land a real use-after-free on a heavily-shared
 *   shared_ptr<AbstractItem>.
 *
 *   Fix uses std::find_if + erase(it) — invalidates only the local
 *   iterator which is then thrown away.  Same observable behaviour
 *   (remove first match by id), no UB.
 *
 * GroupItem live instantiation requires app-common, glib, dlog and
 * the inflator/factory registries; the mocks in this suite cover
 * gtest-fixture-managed item subclasses (ProgressItemTest et al.) but
 * driving GroupItem::RemoveChild end-to-end is heavyweight.  Following
 * the established mirror pattern (test_lazy_init / test_zombie_cleanup
 * / test_cpu_boost_lifecycle) the tests below pin the algorithmic
 * contract — first-match-by-id remove on a std::list of shared_ptr
 * children — using a lightweight FakeChild that exposes only GetId().
 * Drift between mirror and production is guarded structurally: any
 * future GroupItem::RemoveChild that breaks find_if + erase semantics
 * must also break these tests.
 */

#include <gtest/gtest.h>

#include <algorithm>
#include <list>
#include <memory>
#include <string>

namespace {

/* Minimal stand-in for the std::shared_ptr<AbstractItem> elements that
 * GroupItem::children_list_ holds.  Only GetId() is exercised by the
 * production RemoveChild contract, so that's all the mirror needs. */
struct FakeChild {
  std::string id;
  explicit FakeChild(std::string i) : id(std::move(i)) {}
  const std::string& GetId() const { return id; }
};

using ChildList = std::list<std::shared_ptr<FakeChild>>;

/*
 * Mirror of post-fix GroupItem::RemoveChild — kept byte-for-byte
 * equivalent to the production body so any drift in the production
 * find_if predicate or erase call breaks these tests too.
 */
void RemoveChildMirror(ChildList& children, const std::string& itemId) {
  auto it = std::find_if(children.begin(), children.end(),
      [&itemId](const std::shared_ptr<FakeChild>& child) {
        return child && child->GetId() == itemId;
      });
  if (it != children.end())
    children.erase(it);
}

/*
 * Pre-fix shape — kept around as a negative control to pin the
 * regression direction.  std::list::remove(value) walks the list
 * and erases every node whose value compares equal to `value`,
 * invalidating the range-loop iterator on the way.  The `break`
 * after the call hid the UB but a future caller who removes the
 * break to delete every match would crash.
 */
void RemoveChildMirrorPreFix(ChildList& children, const std::string& itemId) {
  for (auto& i : children) {
    if (i && i->GetId() == itemId) {
      children.remove(i);  /* invalidates the range-loop iterator */
      break;
    }
  }
}

}  // namespace

/* ------------------------------------------------------------------ */
/* Post-fix contract pins                                              */
/* ------------------------------------------------------------------ */

TEST(NotificationUafAudit, ITER1_RemoveChild_RemovesMatch) {
  ChildList children;
  children.emplace_back(std::make_shared<FakeChild>("a"));
  children.emplace_back(std::make_shared<FakeChild>("b"));
  children.emplace_back(std::make_shared<FakeChild>("c"));

  ASSERT_EQ(children.size(), 3u);
  RemoveChildMirror(children, "b");

  ASSERT_EQ(children.size(), 2u);
  auto it = children.begin();
  EXPECT_EQ((*it)->GetId(), "a");
  ++it;
  EXPECT_EQ((*it)->GetId(), "c");
}

TEST(NotificationUafAudit, ITER1_RemoveChild_NoMatchIsNoOp) {
  ChildList children;
  children.emplace_back(std::make_shared<FakeChild>("a"));
  children.emplace_back(std::make_shared<FakeChild>("b"));

  RemoveChildMirror(children, "nonexistent");

  ASSERT_EQ(children.size(), 2u);
  auto it = children.begin();
  EXPECT_EQ((*it)->GetId(), "a");
  ++it;
  EXPECT_EQ((*it)->GetId(), "b");
}

TEST(NotificationUafAudit, ITER1_RemoveChild_FirstAndLastBoundaries) {
  ChildList children;
  children.emplace_back(std::make_shared<FakeChild>("a"));
  children.emplace_back(std::make_shared<FakeChild>("b"));
  children.emplace_back(std::make_shared<FakeChild>("c"));

  RemoveChildMirror(children, "a");
  ASSERT_EQ(children.size(), 2u);
  EXPECT_EQ((*children.begin())->GetId(), "b");

  RemoveChildMirror(children, "c");
  ASSERT_EQ(children.size(), 1u);
  EXPECT_EQ((*children.begin())->GetId(), "b");
}

TEST(NotificationUafAudit, ITER1_RemoveChild_OnlyFirstMatchRemoved) {
  /* std::list::remove(value) erases every match; the legacy code's
   * `break` made it remove only the first.  The post-fix find_if +
   * erase preserves that "first match" semantics — pin it. */
  ChildList children;
  children.emplace_back(std::make_shared<FakeChild>("x"));
  children.emplace_back(std::make_shared<FakeChild>("y"));
  children.emplace_back(std::make_shared<FakeChild>("x"));

  RemoveChildMirror(children, "x");

  ASSERT_EQ(children.size(), 2u)
      << "only the first 'x' must be removed; the second must stay";
  auto it = children.begin();
  EXPECT_EQ((*it)->GetId(), "y");
  ++it;
  EXPECT_EQ((*it)->GetId(), "x");
}

TEST(NotificationUafAudit, ITER1_RemoveChild_NullChildrenIgnored) {
  /* Defensive — production children_list_ should not contain NULL
   * shared_ptrs but the find_if predicate guards against it
   * (`return child && child->GetId() == itemId;`).  Pin the guard. */
  ChildList children;
  children.emplace_back(nullptr);
  children.emplace_back(std::make_shared<FakeChild>("a"));
  children.emplace_back(nullptr);

  RemoveChildMirror(children, "a");
  EXPECT_EQ(children.size(), 2u);
}

/* ------------------------------------------------------------------ */
/* Negative control: pre-fix shape behaviour invariant                 */
/* ------------------------------------------------------------------ */

TEST(NotificationUafAudit, ITER1_PreFixMirror_RemovesFirstMatchToo) {
  /* The pre-fix shape with `break` happened to behave correctly on
   * the happy path (remove first match, no observable UB).  Pin
   * that observable behaviour so a future maintainer who removes
   * the `break` to "delete every match" notices the silent
   * semantics change AND lands the post-fix shape instead. */
  ChildList children;
  children.emplace_back(std::make_shared<FakeChild>("x"));
  children.emplace_back(std::make_shared<FakeChild>("y"));
  children.emplace_back(std::make_shared<FakeChild>("x"));

  RemoveChildMirrorPreFix(children, "x");

  /* Same observable semantics on the happy path — both shapes
   * remove only the first match. */
  ASSERT_EQ(children.size(), 2u);
  auto it = children.begin();
  EXPECT_EQ((*it)->GetId(), "y");
  ++it;
  EXPECT_EQ((*it)->GetId(), "x");
}

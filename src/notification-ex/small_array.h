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
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef NOTIFICATION_EX_SMALL_ARRAY_H_
#define NOTIFICATION_EX_SMALL_ARRAY_H_

/*
 * N3-T5 A4: SmallArray<T, N> — stack-buffered scratch array with heap
 * fallback. Used in hot paths where the element count is usually small
 * (< N) but may occasionally exceed it. Avoids the per-call calloc() for
 * the common case.
 *
 * SAFETY CONTRACT:
 *   - data() is valid only while the SmallArray object is alive. Do NOT
 *     return data() or store it past the SmallArray's scope. If the buffer
 *     is on the stack (size() <= N) its contents die with the enclosing
 *     frame — identical to alloca().
 *   - For arrays that must be handed to a C caller as an out parameter
 *     (ownership transfer) use calloc() directly; SmallArray is for
 *     transient intermediate scratch only.
 *
 * Elements are zero-initialised like calloc(), matching the behaviour of
 * the call sites replaced in stub.cc.
 *
 * Internal header — not installed (see CMakeLists.txt PATTERN EXCLUDE).
 */

#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <new>
#include <utility>

namespace notification {
namespace util {

template <typename T, std::size_t N = 16>
class SmallArray {
 public:
  explicit SmallArray(std::size_t n) : size_(n), heap_(nullptr) {
    static_assert(N > 0, "SmallArray inline capacity must be positive");
    if (n <= N) {
      /* stack fast path: zero-initialise the prefix we are going to hand
       * out so callers observe the same semantics as calloc(). */
      std::memset(stack_, 0, n * sizeof(T));
    } else {
      /* heap fallback: calloc matches the out-of-memory-returns-nullptr
       * contract that the replaced sites relied on. */
      heap_ = static_cast<T*>(std::calloc(n, sizeof(T)));
    }
  }

  ~SmallArray() {
    if (heap_ != nullptr)
      std::free(heap_);
  }

  SmallArray(const SmallArray&) = delete;
  SmallArray& operator=(const SmallArray&) = delete;
  SmallArray(SmallArray&&) = delete;
  SmallArray& operator=(SmallArray&&) = delete;

  T* data() {
    if (heap_ != nullptr)
      return heap_;
    if (size_ == 0)
      return nullptr;
    return stack_;
  }

  const T* data() const {
    if (heap_ != nullptr)
      return heap_;
    if (size_ == 0)
      return nullptr;
    return stack_;
  }

  std::size_t size() const { return size_; }

  /* Allocation failure is observable only for the heap path (n > N). */
  bool alloc_ok() const { return size_ <= N || heap_ != nullptr; }

  bool on_stack() const { return heap_ == nullptr && size_ <= N; }

  T& operator[](std::size_t i) { return data()[i]; }
  const T& operator[](std::size_t i) const { return data()[i]; }

 private:
  T stack_[N];
  T* heap_;
  std::size_t size_;
};

}  // namespace util
}  // namespace notification

#endif  // NOTIFICATION_EX_SMALL_ARRAY_H_

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

#ifndef UNIT_TESTS_MOCK_LIBC_CALLOC_MOCK_H_
#define UNIT_TESTS_MOCK_LIBC_CALLOC_MOCK_H_

#include <cstddef>

// One-shot size-keyed calloc OOM injection.
// Arms the interceptor to fail the next calloc(nmemb, size) where
// nmemb*size == target_size, then auto-disarms.
// NOT thread-safe; use only from single-threaded test code.
void calloc_set_fail_on_size(std::size_t target_size);
void calloc_reset_fail();

// Call-counter malloc OOM injection.
// Arms the interceptor to fail the (call_index)th malloc call after arming
// (0 = fail the very next call). One-shot; auto-disarms after firing.
// Use only when _no_ malloc calls happen between arming and the target line.
// NOT thread-safe.
void malloc_set_fail_on_call(int call_index);
void malloc_reset_fail();

#endif  // UNIT_TESTS_MOCK_LIBC_CALLOC_MOCK_H_

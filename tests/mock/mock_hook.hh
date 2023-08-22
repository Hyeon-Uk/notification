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

#ifndef UNIT_TESTS_MOCK_MOCK_HOOK_H_
#define UNIT_TESTS_MOCK_MOCK_HOOK_H_

#define MOCK_HOOK_P0(MOCK_CLASS, f)                                            \
    TestFixture::GetMock<MOCK_CLASS>().f()
#define MOCK_HOOK_P1(MOCK_CLASS, f, p1)                                        \
    TestFixture::GetMock<MOCK_CLASS>().f(p1)
#define MOCK_HOOK_P2(MOCK_CLASS, f, p1, p2)                                    \
    TestFixture::GetMock<MOCK_CLASS>().f(p1, p2)
#define MOCK_HOOK_P3(MOCK_CLASS, f, p1, p2, p3)                                \
    TestFixture::GetMock<MOCK_CLASS>().f(p1, p2, p3)
#define MOCK_HOOK_P4(MOCK_CLASS, f, p1, p2, p3, p4)                            \
    TestFixture::GetMock<MOCK_CLASS>().f(p1, p2, p3, p4)
#define MOCK_HOOK_P5(MOCK_CLASS, f, p1, p2, p3, p4, p5)                        \
    TestFixture::GetMock<MOCK_CLASS>().f(p1, p2, p3, p4, p5)
#define MOCK_HOOK_P6(MOCK_CLASS, f, p1, p2, p3, p4, p5, p6)                    \
    TestFixture::GetMock<MOCK_CLASS>().f(p1, p2, p3, p4, p5, p6)
#define MOCK_HOOK_P7(MOCK_CLASS, f, p1, p2, p3, p4, p5, p6, p7)                \
    TestFixture::GetMock<MOCK_CLASS>().f(p1, p2, p3, p4, p5, p6, p7)
#define MOCK_HOOK_P8(MOCK_CLASS, f, p1, p2, p3, p4, p5, p6, p7, p8)            \
    TestFixture::GetMock<MOCK_CLASS>().f(p1, p2, p3, p4, p5, p6, p7, p8)
#define MOCK_HOOK_P10(MOCK_CLASS, f, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10)  \
    TestFixture::GetMock<MOCK_CLASS>().f(                                      \
        p1, p2, p3, p4, p5, p6, p7, p8, p9, p10)
#define MOCK_HOOK_P11(MOCK_CLASS, f, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11)  \
    TestFixture::GetMock<MOCK_CLASS>().f(                                      \
        p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11)

#endif  // UNIT_TESTS_MOCK_MOCK_HOOK_H_

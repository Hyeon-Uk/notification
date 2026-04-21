# ADR-003: 삼성 TV AI OS — "Agent AI 기반 Friction-less TV 커머스 & 패키지 광고" 시나리오의 Capability 아키텍처

**Status:** Proposed **Date:** 2026-04-22 **Deciders:** AI OS Platform / TV SW / Ads Platform / Commerce & Payments / Legal & Privacy 팀 **Scope:** 삼성 TV AI OS가 **에이전트 자체를 광고 상품으로 판매**하고, **Shoppable 광고의 상품 선택·결제를 자동화**하여 "리모컨 몇 번"이 아닌 **대화 몇 마디**로 TV에서 구매가 완결되게 한다.

---

## 1. Context

### 1.1 두 개의 서브 시나리오

|#|시나리오|구동 방식|
|---|---|---|
|**A**|**Agent-as-Ad**: "결혼 기념일 기념 부쉐론 반지 선물해보세요."|브랜드가 **에이전트 슬롯**을 구매 → 사용자 맥락(기념일)과 매칭 시 에이전트가 선제 제안|
|**B**|**Shoppable Ad 자동화**: "Home Depot와 함께 봄맞이 정원 꾸미기"|광고 크리에이티브에 **패키지 상품(Bundle)** 이 연결 → 사용자가 "이거 주문해줘" 말하면 에이전트가 큐레이션·결제까지 자동|

### 1.2 공통 파이프라인 (5단계)

|단계|Phase|설명|
|---|---|---|
|① Trigger|에이전트 활성화|맥락(날짜·대화·시청) 또는 광고 노출 시 제안 트리거|
|② Understand|의도·제약 파악|예산, 수령인, 배송 기한, 선호 브랜드, 동의 범위|
|③ Compose|패키지 구성|단일 SKU + 부속(포장·각인·설치·배송일) 을 한 묶음(Bundle)으로|
|④ Transact|결제·확정|본인확인 · 결제 · 배송지 · 영수증 · 취소창|
|⑤ Aftercare|사후 관리|추적·반품·후속 제안·광고주 측 전환 리포트|

### 1.3 비기능 요구 (NFR)

|항목|목표|
|---|---|
|결제 완료까지 대화 턴|≤ 3턴 (동의·결제·확인)|
|제안→장바구니 레이턴시|≤ 3s|
|결제 단계 본인확인|음성ID + (모바일 biometric **또는** 원격 PIN)|
|프라이버시|커머스/결제 의도는 온디바이스 우선 처리, 광고 타겟 신호는 집계(aggregate)로만|
|규제|고가/주류/처방 제품 등 특수 카테고리 차단 테이블|
|광고 적합성|Brand Safety · Household 동거인/미성년자 보호 필수|

### 1.4 Capability 3종 재확인

|종류|용도 예|
|---|---|
|**App Action**|`samsung-pay.checkout`, `boucheron.catalog.get`, `home-depot.cart.createBundle`, `calendar.today`|
|**CLI Tool**|`intent-classify`, `price-compare`, `risk-score`, `receipt-gen`, `ads-auction-cli`|
|**SKILL**|`gift-concierge`, `bundle-composer`, `checkout-captain`, `ad-consent-guard`|

> **원칙**: **돈이 움직이는 모든 스텝은 Action**(감사·규제 대응). **랭킹·제안·대화는 SKILL**. **결정적 유틸/계산은 CLI**.

---

## 2. 필요 기술 도출 (Capability Inventory)

### Phase ① Trigger — 에이전트 활성화 / 광고 노출

|Capability|종류|설명|
|---|---|---|
|`calendar.upcoming(days=30)`|**App Action**|기념일·생일·결혼기념일 같은 트리거 이벤트|
|`ads-slot.bid(context)`|**App Action** (Ads Platform)|현재 컨텍스트(광고 재고 슬롯)에 실시간 입찰|
|`ads-agent-slot.serve(userCtx)`|**App Action**|**에이전트 슬롯** (시나리오 A) 서빙 — 일반 영상광고와 구분|
|`scene-understand`|**SKILL**|현재 시청중인 장면(집·정원·주방)에서 Shoppable 광고 맥락 추출|
|`consent-state.get(userId)`|**App Action** (AI OS Privacy)|개인화 광고/커머스 동의 여부|
|`household-profile.current()`|**App Action**|현재 시청자 프로파일(부모/아이/게스트)|
|`intent-classify`|**CLI Tool**|"하나 사줘" vs "관심만" vs "나중에" 분류 결정성|

### Phase ② Understand — 의도·제약 파악

|Capability|종류|설명|
|---|---|---|
|`gift-concierge` (시나리오 A)|**SKILL**|수령인 · 관계 · 예산 · 톤 · 포장 옵션 대화 수집|
|`space-profile.get(roomId)` (시나리오 B)|**App Action** (SmartThings)|거주 공간 크기/조도/기존 가구 힌트|
|`budget-guard`|**SKILL**|예산 상한·기본값 검증 + 과도결제 경고|
|`region-rule`|**CLI Tool**|지역 규제·연령 제한 카테고리 조회|
|`ad-consent-guard`|**SKILL**|개인화 광고 동의 범위 내에서만 제안|
|`address-book.get(recipient)`|**App Action** (Samsung Contacts)|수령인 주소·전화 (수신인 동의 전제)|

### Phase ③ Compose — 패키지(Bundle) 구성

|Capability|종류|설명|
|---|---|---|
|`merchant.catalog.search(query)`|**App Action** (Boucheron / Home Depot / …)|브랜드 카탈로그|
|`bundle-composer`|**SKILL**|SKU + 각인·포장·배송·설치·보증 확장 옵션을 하나의 Bundle로 구성|
|`price-compare`|**CLI Tool**|동일 SKU 가맹점 비교 (광고주 합의 범위 내)|
|`inventory.check`|**App Action**|재고·지역별 배송 가능 여부|
|`ar-preview.show(asset)`|**App Action** (시나리오 B)|AR/시뮬레이션 — 거실 가든 배치 프리뷰|
|`license-guard`|**SKILL**|광고 규정·카피·가격표시법 준수 (예: "최대 할인" 문구)|
|`coupon-apply`|**CLI Tool**|결정적 쿠폰 적용(중복 적용 규칙)|

### Phase ④ Transact — 결제·확정

|Capability|종류|설명|
|---|---|---|
|`samsung-pay.checkout(bundleId)`|**App Action**|카드·간편결제·BNPL 토큰 기반|
|`3ds.verify(transactionId)`|**App Action**|고액/신규 카드 3-D Secure|
|`voice-id.verify(userId)`|**App Action**|음성 본인확인(낮은 friction)|
|`mobile-biometric.handoff(txId)`|**App Action**|폰으로 지문/Face ID — QR 또는 push|
|`risk-score`|**CLI Tool**|디바이스/IP/시간/금액 기반 결정적 리스크|
|`checkout-captain`|**SKILL**|주문·결제·배송지·쿠폰을 한 대화로 묶는 오케스트레이터|
|`receipt-gen`|**CLI Tool**|영수증·주문요약 서명된 PDF|
|`cancel-window.register`|**CLI Tool** (`tv-scheduler`)|N분 내 원터치 취소 가능하게 타이머 등록|

### Phase ⑤ Aftercare — 사후

|Capability|종류|설명|
|---|---|---|
|`order.track(orderId)`|**App Action**|배송 상태 구독|
|`notification.push`|**App Action**|"수령 완료", "반품 가능 기한"|
|`return.start(orderId)`|**App Action**|반품 개시|
|`post-purchase-suggest`|**SKILL**|보완 제품 제안 (주류·담배류 제외)|
|`attribution.report`|**App Action** (Ads Platform)|광고주 측 전환 리포트(집계·ID-less)|
|`household-share.propose`|**App Action**|가족과 선물 스케줄/카드 메시지 공유|

---

## 3. 각각의 구조 (Structure)

### 3.1 전체 플로우

```
 ┌──────────────────────┐          ┌──────────────────────────┐
 │  맥락 신호 수집      │          │  광고 인벤토리 & 에이전트 │
 │  (시청/달력/대화)    │◄────────►│  슬롯 입찰 (Ads Platform) │
 └──────────┬───────────┘          └──────────────┬───────────┘
            ▼                                      ▼
     ┌───────────────────────────────────────────────────┐
     │  AI OS Orchestrator — Intent · Consent · Policy   │
     └──────────────┬────────────────────────────────────┘
                    ▼
      ┌────────────────────────────────────┐
      │  SKILLs                            │
      │  gift-concierge / bundle-composer  │
      │  checkout-captain / ad-consent-guard│
      └─────┬────────────┬─────────────────┘
            ▼            ▼
       App Actions    CLI Tools
     (Merchant,     (price-compare,
      Pay, Ads,      risk-score,
      Calendar)      receipt-gen)
            │            │
            ▼            ▼
      Brand Apps / Samsung Pay / SmartThings / Ads Platform
```

### 3.2 App Action — **결제/광고** 전용 보강 규약

**보강 필드**

```yaml
id: samsung-pay.checkout
version: 3.1.0
description: Bundle 결제 처리 (Samsung Pay 토큰 기반)
permissions: [PAYMENT, BIOMETRIC, USER_IDENTITY]
input:
  bundleId:     { type: string }
  paymentMethodId: { type: string, optional: true }
  shippingAddressId: { type: string }
  idempotencyKey: { type: string, required: true }   # 중복 결제 방지
output:
  orderId:      { type: string }
  totalAmount:  { type: money }
  cancelableUntil: { type: string, format: datetime }
sla:
  p95LatencyMs: 1500
compliance:
  kycRequired:   conditional
  amlScreening:  true
  threeDSeCURE:  amountGt: 500
  auditLog:      mandatory
```

**런타임 규약**

- **멱등 키 필수**: 대화 재시도로 인한 이중 결제를 원천 차단.
- **2-party 확인**: 음성ID + (모바일 biometric | PIN) — 둘 중 하나는 반드시 onboarding된 채널.
- **취소창(Cancel Window)**: `cancelableUntil` 정보를 UI에 노출하고 `cancel-window.register`로 원터치 취소 보장.
- **광고 전환 콜백**: `attribution.report`는 결제 완료 후 _집계된 토큰_만 광고주에게 전달.

---

### 3.3 CLI Tool 구조 — 커머스/광고 특화

```bash
# 의도 분류 (결정적 · 감사가능)
$ intent-classify --text "반지 하나 사줘" --context gift --json
{ "intent": "purchase", "confidence": 0.94, "slots": { "category": "ring" } }

# 결정적 가격비교 (광고주 합의 범위 내)
$ price-compare --sku BOUCHERON-Q12 --region KR --merchants boucheron,sn --json

# 리스크 스코어링
$ risk-score --amount 3200000 --currency KRW --userId ux --deviceTrust 0.8 --json
{ "score": 0.12, "requiresStepUp": false }

# 영수증 생성 (서명된 PDF)
$ receipt-gen --orderId O-9981 --out /out/r.pdf --sign tpm --json

# 광고 경매 (에이전트 슬롯)
$ ads-auction-cli --slot AGENT_SLOT_PRIME --ctx '{ "event": "anniversary", "budget": 200000 }' --json
```

**공통 규약**: JSON 전용, `--trace-id`, `--dry-run`, 에러코드(권한/타임아웃/규제/리스크) 테이블 유지.

---

### 3.4 SKILL 구조

**`gift-concierge` (시나리오 A 핵심 SKILL)**

```markdown
# SKILL: gift-concierge

## When to trigger
- calendar.upcoming에 anniversary/birthday/holiday 발견
- 또는 ads-agent-slot.serve 가 gift-intent 컨텍스트로 노출

## Inputs
- eventType (anniversary | birthday | …)
- recipient (관계/나이/연락 가능 여부)
- budget (명시/추정)
- time-to-event (오늘? D-7?)

## Procedure
1. SKILL: ad-consent-guard → 개인화 제안 허용 여부 체크
2. CLI: intent-classify → 구매 의향/관심 단계 구분
3. Action: household-profile.current → 미성년/게스트면 고가제안 중단
4. Action: merchant.catalog.search (3개 브랜드 병렬)
5. SKILL: bundle-composer → SKU + 각인·포장·익일배송 Bundle 구성
6. Dialog(짧은 3턴): 수령인 확인 → 메시지 카드 → 결제 방법
7. SKILL: checkout-captain → 결제 단계로 위임

## Guardrails
- 예산 미명시 시 사용자 과거 지출 상위 15% 이내 기본값 제안
- 결혼기념일 같은 *비밀 성격* 이면 household 공유 자동 보류
```

**`bundle-composer` (시나리오 B 핵심 SKILL)**

```markdown
# SKILL: bundle-composer

## Goal
단일 SKU 대신 "정원 꾸미기 키트"처럼 **여러 SKU + 옵션(배송일/설치)** 을 하나의 Bundle로.

## Procedure
1. scene-understand / space-profile.get → 공간 맥락
2. merchant.catalog.search (Home Depot) + inventory.check
3. CLI: price-compare + coupon-apply (광고주 합의 쿠폰만)
4. AR: ar-preview.show (선택형) → 사용자 확인
5. SKILL: license-guard → 가격 표기 규정 통과
6. return Bundle{items[], price, shipSLA, installOption}
```

**`checkout-captain`**

```markdown
# SKILL: checkout-captain

## Goal
대화 3턴(확인·결제·알림) 이내 결제 완결.

## Procedure
1. Action: risk-score → stepUp 필요 여부
2. Action: voice-id.verify + (필요시) mobile-biometric.handoff
3. Action: samsung-pay.checkout(bundleId, idempotencyKey)
4. Action: cancel-window.register (기본 10분, 카테고리별 확장)
5. Action: receipt-gen + notification.push

## Guardrails
- 실패/타임아웃 재시도는 **동일 idempotencyKey**로만
- 취소창 내 cancel은 친화 UI(원탭) + TTS 확인
- 광고주 전환 이벤트는 결제 성공 후에만, 집계 토큰으로
```

**`ad-consent-guard`** — 광고주 계약 ↔ 사용자 동의 ↔ 지역 규제 교집합만 허용.

---

### 3.5 엔드투엔드 시퀀스 — 시나리오 A (Agent-as-Ad: 부쉐론)

```
t0   calendar.upcoming → 결혼기념일 D-5 발견
t0   ads-slot.bid (AGENT_SLOT) 경매 → Boucheron 낙찰
t0+1 household-profile.current → 본인만 시청, ad-consent 허용
t0+1 에이전트 카드: "결혼기념일이 곧이에요. 부쉐론 추천을 보여드릴까요?"
턴1  사용자: "응, 보여줘"
      Action: merchant.catalog.search(boucheron, ring, 성향기반 필터)
      SKILL: bundle-composer (반지 + 각인 + 익일 포장배송)
턴2  사용자: "이걸로, 각인은 A♥B, 내일 도착"
      SKILL: checkout-captain
         risk-score → stepUp=true (고액)
         voice-id.verify OK → mobile-biometric.handoff (폰에서 지문)
         samsung-pay.checkout (idempotencyKey)
턴3  "결제 완료, 10분 내 취소 가능. 배송 알림은 폰으로 드릴게요."
후속 attribution.report (집계, PII 없음) → 광고주 대시보드
```

### 3.6 엔드투엔드 시퀀스 — 시나리오 B (Shoppable: Home Depot 봄맞이 정원)

```
t0   광고 재생 중 `scene-understand` → "가든 봄맞이" 컨텍스트
     오버레이: "이 광고의 상품을 에이전트가 세트로 준비해드릴까요?"
턴1  "응, 앞마당 10평에 맞춰서"
      space-profile.get(roomId=frontyard)
      bundle-composer → 잔디씨/비료/스프링클러/수전/설치 서비스
      ar-preview.show → 사용자 확인
턴2  "설치 서비스 포함해서, 다음 주 토요일 오전"
      checkout-captain → risk-score 낮음 → voice-id only
      samsung-pay.checkout
턴3  "주문 확정, 토요일 도착·설치. 일정 폰 캘린더에 추가했어요."
```

---

## 4. Options Considered

### Option A — **Deep link 중심** (브랜드 앱으로 이동 후 결제)

|차원|평가|
|---|---|
|구현 복잡도|하|
|Friction|상 (앱 이동·재로그인)|
|전환율|하|
|광고주 통제|상|

### Option B — **TV OS 단일 결제 위젯** (모든 브랜드가 공용 결제)

|차원|평가|
|---|---|
|Friction|하|
|브랜드 차별화|하|
|법적 리스크|중 (PG·KYC 단일주체화)|

### Option C (권장) — **하이브리드**: _브랜드 Action으로 카탈로그·Bundle, Samsung Pay Action으로 결제, 에이전트 슬롯은 별도 광고 상품_

- 브랜드는 Catalog / Cart / Inventory Action만 구현, 결제는 OS 표준 Action.
- "Agent Slot"을 **일반 영상광고와 다른 인벤토리**로 분리해 가격·보고·규정 독립.
- AR·시뮬 같은 고가치 기능은 브랜드 선택 옵션.

---

## 5. Trade-off Analysis

|축|선택|이유|
|---|---|---|
|결제 주체|Samsung Pay 단일(Action)|이중 결제·사기·정산 단순화|
|본인확인|음성ID + (모바일 biometric 또는 PIN) 2중|UX 낮은 friction + 부정사용 방지|
|광고 타겟|Opt-in + 집계 신호|맞춤성과 프라이버시 균형|
|취소창|기본 10분, 카테고리별 확장(사치/가구 등)|소비자 보호 + 판매자 보호 균형|
|에이전트 슬롯 가격 결정|이벤트·맥락 기반 실시간 경매|가치 기반 가격 · 유휴 슬롯 최소화|
|가족 TV 상 성인 제품|기본 차단 + 본인 모드에서만 노출|가정 안전|
|광고주 쿠폰|allowlist + 결정적 적용|오용/중복 방지|
|Bundle 표기|"광고" 라벨 필수 + 내역 투명|광고법 준수|

---

## 6. Consequences

**쉬워지는 것**

- 브랜드 추가 = Catalog/Cart/Inventory Action 3개만 등록하면 즉시 Shoppable.
- 새 "Agent 광고 상품" 포맷(여행, 선물, 시즌 리모델링)을 SKILL 변형으로 빠르게 런칭.
- 공통 `checkout-captain` 때문에 모든 시나리오에서 결제 UX 일관.

**어려워지는 것**

- 광고/상거래/KYC/AML 규정이 국가별 상이 → `region-rule` 정책 테이블이 커짐.
- 광고주와의 가격표시·쿠폰 계약을 **결정적으로** 실행하는 `coupon-apply`의 검증 부담.
- 가정 공용 TV의 다중 사용자 구분(부모·자녀)을 **고가 결제에서 오판 없이** 하는 게 어려움.

**나중에 다시 볼 것**

- 에이전트 슬롯의 _Brand Safety_ 자동화 품질 (오프닝 문장이 브랜드 가이드 위반 없는지).
- 온디바이스 LLM만으로 대화 3턴 안에 결제 종결 가능한 카테고리 커버리지.
- 지역 규제 변화(AI 광고 공시·쿠팡식 정산)에 대응한 정책 모듈화.

---

## 7. Action Items

1. [ ] `samsung-pay.checkout` v3.x — 멱등키·3DS·취소창·감사 스키마 확정
2. [ ] `ads-agent-slot.serve` Action 스펙 + 일반 광고 인벤토리와의 격리
3. [ ] `gift-concierge` / `bundle-composer` / `checkout-captain` SKILL PoC 및 평가세트
4. [ ] `intent-classify` / `risk-score` / `ads-auction-cli` / `receipt-gen` CLI 구현
5. [ ] 가족 프로파일 × 고가 결제 UX 정책(본인 모드 진입 플로우)
6. [ ] `region-rule` 초기 테이블(KR/US/JP/EU): 주류·의약·금융·사치 카테고리
7. [ ] 광고주 전환 리포트 프로토콜 (집계·ID-less)
8. [ ] Legal 리뷰: 광고 라벨 · 쿠폰 표시 · 미성년자 보호 · 취소권

---

## 8. ADR-001 / ADR-002 과의 재사용

|재사용 Capability|커머스 시나리오에서의 용도|
|---|---|
|`calendar.today` / `.upcoming` (ADR-001)|기념일/시즌 트리거|
|`dashboard.compose` (ADR-001)|구매 후 홈에 "주문 위젯" 표시|
|`notification.push` / `ambient.announce` (ADR-001)|결제 확인·취소창 안내|
|`scene-understand` (ADR-002)|Shoppable 광고 맥락 추출|
|`license-guard` (ADR-002)|광고 카피·가격표시 규정 준수|
|`privacy-guard` (ADR-001)|가족 TV 개인화 노출 제어|
|`presence-get` (ADR-001)|고가 결제 단계에서 실제 시청자 확인 보조|

→ **ADR-003은 결제·광고 전용 Action 집합만 신규이고**, 오케스트레이션·프라이버시·가드는 앞의 두 ADR 컴포넌트를 재사용해 일관된 AI OS 기반을 유지한다.
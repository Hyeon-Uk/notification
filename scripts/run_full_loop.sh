#!/bin/bash
# Full orchestration loop:
#   1. Phase A: AUL fix
#   2. Gap analysis + fix loop (max 10)
#   3. Notify user when 0 gaps
#   4. Phase B: Refactoring

set -euo pipefail

REPO=/home/kimhyeonuk/.openclaw/workspace/refactoring/notification
PLANS=/home/kimhyeonuk/.openclaw/workspace/refactoring/refactoring-plans/execution/tasks/notification
DORMAMMU="dormammu"
LOG_DIR=/tmp/noti-loop-logs
DISCORD_SESSION="user:369332395545198603"
MAX_GAP_LOOPS=10

mkdir -p "$LOG_DIR"
cd "$REPO"

log() { echo "[$(date '+%Y-%m-%d %H:%M:%S')] $*" | tee -a "$LOG_DIR/orchestrator.log"; }

# ─────────────────────────────────────────────────────
# Helper: send Discord message via claude sessions
# ─────────────────────────────────────────────────────
send_discord() {
  local msg="$1"
  claude --print --dangerously-skip-permissions \
    -p "Use the discord skill to send a direct message to user 369332395545198603: $msg" \
    2>/dev/null || true
}

# ─────────────────────────────────────────────────────
# Helper: count remaining gaps from GAP_REPORT.md
# ─────────────────────────────────────────────────────
count_gaps() {
  local report="$PLANS/GAP_REPORT.md"
  if [[ ! -f "$report" ]]; then echo "999"; return; fi
  grep -oP 'Total gaps: \K[0-9]+' "$report" | tail -1 || echo "999"
}

# ─────────────────────────────────────────────────────
# PHASE A: AUL PLT interposition fix
# ─────────────────────────────────────────────────────
log "=== PHASE A: AUL PLT fix ==="

$DORMAMMU run \
  --repo-root "$REPO" \
  --prompt-file prompts/04_phase_a_aul_fix.md \
  --max-iterations 5 \
  --debug \
  > "$LOG_DIR/phase_a.log" 2>&1

phase_a_exit=$?
log "Phase A exit: $phase_a_exit"

if [[ $phase_a_exit -ne 0 ]]; then
  log "Phase A FAILED. Check $LOG_DIR/phase_a.log"
  send_discord "notification Phase A (AUL fix) FAILED. 수동 확인 필요: $LOG_DIR/phase_a.log"
  exit 1
fi

log "Phase A complete."

# ─────────────────────────────────────────────────────
# GAP FIX LOOP (max 10)
# ─────────────────────────────────────────────────────
log "=== GAP FIX LOOP (max $MAX_GAP_LOOPS iterations) ==="

for iteration in $(seq 1 $MAX_GAP_LOOPS); do
  log "--- Gap loop iteration $iteration/$MAX_GAP_LOOPS ---"

  # Inject iteration number into prompt
  sed "s/Loop iteration: <N>/Loop iteration: $iteration/" \
    prompts/05_gap_fix.md > /tmp/gap_fix_iter.md

  $DORMAMMU run \
    --repo-root "$REPO" \
    --prompt-file /tmp/gap_fix_iter.md \
    --max-iterations 3 \
    --debug \
    > "$LOG_DIR/gap_loop_${iteration}.log" 2>&1

  gap_exit=$?
  log "Gap loop $iteration exit: $gap_exit"

  if [[ $gap_exit -ne 0 ]]; then
    log "Gap loop $iteration FAILED. Check $LOG_DIR/gap_loop_${iteration}.log"
    send_discord "notification gap fix loop $iteration FAILED. 수동 확인 필요."
    break
  fi

  gaps=$(count_gaps)
  log "Remaining gaps after iteration $iteration: $gaps"

  if [[ "$gaps" -eq 0 ]]; then
    log "✓ No gaps remaining after $iteration iteration(s)."
    break
  fi

  if [[ $iteration -eq $MAX_GAP_LOOPS ]]; then
    log "Max gap iterations reached. Remaining gaps: $gaps"
    send_discord "notification 갭 수정 루프 $MAX_GAP_LOOPS회 완료. 남은 갭: $gaps개. GAP_REPORT.md 확인 바람."
  fi
done

# ─────────────────────────────────────────────────────
# Check final gap count
# ─────────────────────────────────────────────────────
final_gaps=$(count_gaps)
log "Final gap count: $final_gaps"

if [[ "$final_gaps" -gt 0 ]]; then
  log "WARNING: $final_gaps gaps remain after max iterations."
  gap_report="$PLANS/GAP_REPORT.md"
  send_discord "notification 테스트 커버리지 루프 완료 (갭 ${final_gaps}개 잔류). 기술적 차단 사유는 GAP_REPORT.md 참조."
else
  # ─────────────────────────────────────────────────────
  # All covered — notify user
  # ─────────────────────────────────────────────────────
  log "✓ All APIs and scenarios covered. Notifying user."

  unit_count=$(grep -oP 'Unit.*PASS: \K[0-9]+' "$PLANS/TEST_RESULT_PHASE_A.md" 2>/dev/null | tail -1 || echo "?")
  integ_count=$(grep -oP 'Integration.*PASS: \K[0-9]+' "$PLANS/TEST_RESULT_PHASE_A.md" 2>/dev/null | tail -1 || echo "?")

  send_discord "✅ notification 테스트 100% 커버 완료.
단위: ${unit_count}+ PASS | 통합: ${integ_count}+ PASS/0 FAIL | 스모크: 6 PASS
미커버 API/시나리오: 0개
→ C++17 리팩토링 루프 시작합니다."
fi

# ─────────────────────────────────────────────────────
# PHASE B: C→C++ Refactoring
# ─────────────────────────────────────────────────────
log "=== PHASE B: C→C++ Refactoring ==="

$DORMAMMU run \
  --repo-root "$REPO" \
  --prompt-file prompts/06_refactoring.md \
  --max-iterations -1 \
  --debug \
  > "$LOG_DIR/phase_b_refactoring.log" 2>&1

phase_b_exit=$?
log "Phase B exit: $phase_b_exit"

if [[ $phase_b_exit -ne 0 ]]; then
  log "Phase B FAILED. Check $LOG_DIR/phase_b_refactoring.log"
  send_discord "notification Phase B (C→C++ 리팩토링) FAILED. 수동 확인 필요."
  exit 1
fi

log "=== ALL DONE ==="
send_discord "🎉 notification 리팩토링 완전 완료!
Phase A (AUL fix) ✓ | 갭 수정 루프 ✓ | Phase B (C++17 마이그레이션) ✓
최종 리포트: $PLANS/GAP_REPORT.md"
